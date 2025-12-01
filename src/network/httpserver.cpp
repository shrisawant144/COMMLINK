#include "commlink/network/httpserver.h"
#include <QDateTime>
#include <QRegularExpression>

HttpServer::HttpServer(QObject *parent)
    : QObject(parent), m_format(DataFormatType::JSON), m_sslEnabled(false) {
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &HttpServer::onNewConnection);
}

bool HttpServer::startServer(quint16 port) {
    if (m_server->listen(QHostAddress::Any, port)) {
        return true;
    }
    emit errorOccurred(m_server->errorString());
    return false;
}

void HttpServer::stopServer() {
    for (auto socket : m_clients.keys()) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    m_clients.clear();
    m_server->close();
}

bool HttpServer::isListening() const {
    return m_server->isListening();
}

void HttpServer::onNewConnection() {
    if (m_clients.size() >= MAX_CLIENTS) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        socket->disconnectFromHost();
        socket->deleteLater();
        emit errorOccurred("Max client limit reached. Connection refused.");
        return;
    }
    QTcpSocket *socket = m_server->nextPendingConnection();
    QString clientInfo = socket->peerAddress().toString() + ":" + QString::number(socket->peerPort());
    m_clients[socket] = clientInfo;
    connect(socket, &QTcpSocket::readyRead, this, &HttpServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &HttpServer::onClientDisconnected);
    emit clientConnected(clientInfo);
}

void HttpServer::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    if (data.isEmpty()) {
        emit errorOccurred("Failed to read data from client: " + socket->peerAddress().toString());
        return;
    }
    m_requestBuffers[socket].append(data);
    // Try to parse complete requests
    while (tryParseCompleteRequest(socket)) {
        // Continue processing if there are more complete requests
    }
}

void HttpServer::onClientDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString clientInfo = m_clients.take(socket);
    m_requestBuffers.remove(socket);
    emit clientDisconnected(clientInfo);
    socket->deleteLater();
}

DataFormatType HttpServer::detectContentType(const QString& contentType) {
    QString ct = contentType.toLower().split(';').first().trimmed();
    
    if (ct == "application/json") return DataFormatType::JSON;
    if (ct == "application/xml" || ct == "text/xml") return DataFormatType::XML;
    if (ct == "text/csv") return DataFormatType::CSV;
    if (ct == "text/plain") return DataFormatType::TEXT;
    if (ct == "application/octet-stream") return DataFormatType::BINARY;
    
    return m_format;
}

DataFormatType HttpServer::detectAcceptType(const QString& accept) {
    QString acc = accept.toLower().split(';').first().split(',').first().trimmed();
    
    if (acc == "application/json") return DataFormatType::JSON;
    if (acc == "application/xml" || acc == "text/xml") return DataFormatType::XML;
    if (acc == "text/csv") return DataFormatType::CSV;
    if (acc == "text/plain") return DataFormatType::TEXT;
    if (acc == "application/octet-stream") return DataFormatType::BINARY;
    if (acc == "*/*" || acc.isEmpty()) return m_format;
    
    return m_format;
}

QByteArray HttpServer::buildResponseBody(const HttpRequest& request, DataFormatType format) {
    switch (format) {
        case DataFormatType::JSON: {
            QString jsonResponse = R"({"status":"received","method":")" + request.method + R"(","path":")" + request.path + R"("})";
            return jsonResponse.toUtf8();
        }
        case DataFormatType::XML: {
            QString xmlResponse = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<response>\n  <status>received</status>\n  <method>" + 
                                 request.method + "</method>\n  <path>" + request.path + "</path>\n</response>";
            return xmlResponse.toUtf8();
        }
        case DataFormatType::CSV: {
            QString csvResponse = "status,method,path\nreceived," + request.method + "," + request.path;
            return csvResponse.toUtf8();
        }
        case DataFormatType::TEXT: {
            QString textResponse = "Status: received\nMethod: " + request.method + "\nPath: " + request.path;
            return textResponse.toUtf8();
        }
        case DataFormatType::BINARY:
        case DataFormatType::HEX:
        default:
            return request.body;
    }
}

bool HttpServer::tryParseCompleteRequest(QTcpSocket* socket) {
    QByteArray& buffer = m_requestBuffers[socket];
    
    // Find the end of headers (\r\n\r\n)
    int headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd == -1) {
        return false; // Headers not complete yet
    }
    
    // Parse headers
    QString headerData = QString::fromUtf8(buffer.left(headerEnd));
    QStringList lines = headerData.split("\r\n");
    
    if (lines.isEmpty()) {
        return false;
    }
    
    // Parse request line
    QStringList requestLine = lines[0].split(" ");
    if (requestLine.size() < 2) {
        return false;
    }
    
    HttpRequest request;
    request.method = requestLine[0];
    request.path = requestLine[1];
    
    // Parse headers
    for (int i = 1; i < lines.size(); ++i) {
        QString line = lines[i];
        int colonIndex = line.indexOf(':');
        if (colonIndex != -1) {
            QString key = line.left(colonIndex).trimmed();
            QString value = line.mid(colonIndex + 1).trimmed();
            request.headers[key] = value;
        }
    }
    
    // Check for Content-Length
    int contentLength = 0;
    if (request.headers.contains("Content-Length")) {
        contentLength = request.headers["Content-Length"].toInt();
    }
    
    int bodyStart = headerEnd + 4; // After \r\n\r\n
    if (buffer.size() < bodyStart + contentLength) {
        return false; // Body not complete yet
    }
    
    // Extract body
    request.body = buffer.mid(bodyStart, contentLength);
    
    // Remove processed request from buffer
    buffer.remove(0, bodyStart + contentLength);
    
    // Process the request
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString source = m_clients[socket] + " [" + request.method + " " + request.path + "]";
    
    // Handle OPTIONS request for CORS preflight
    if (request.method == "OPTIONS") {
        QByteArray response = buildCORSPreflightResponse();
        socket->write(response);
        socket->flush();
        return true;
    }
    
    // Detect format from Content-Type header if available
    DataFormatType requestFormat = m_format;
    if (request.headers.contains("Content-Type")) {
        requestFormat = detectContentType(request.headers["Content-Type"]);
    }
    
    DataMessage msg = DataMessage::deserialize(request.body, requestFormat);
    emit messageReceived(msg, source, timestamp);
    
    // Create response in the same format as the request (or use Accept header if provided)
    DataFormatType responseFormat = requestFormat;
    if (request.headers.contains("Accept")) {
        responseFormat = detectAcceptType(request.headers["Accept"]);
    }
    
    // Build response message based on format
    QByteArray responseBody = buildResponseBody(request, responseFormat);
    
    static const int HTTP_OK = 200;
    QByteArray response = buildResponse(HTTP_OK, responseBody, responseFormat);
    socket->write(response);
    socket->flush();
    
    return true;
}

QByteArray HttpServer::buildResponse(int statusCode, const QByteArray& body, DataFormatType format) {
    static const int HTTP_OK = 200;
    QString statusText = (statusCode == HTTP_OK) ? "OK" : "Error";
    
    // Determine Content-Type based on format
    QString contentType;
    switch (format) {
        case DataFormatType::JSON:
            contentType = "application/json";
            break;
        case DataFormatType::XML:
            contentType = "application/xml";
            break;
        case DataFormatType::CSV:
            contentType = "text/csv";
            break;
        case DataFormatType::TEXT:
            contentType = "text/plain; charset=utf-8";
            break;
        case DataFormatType::BINARY:
            contentType = "application/octet-stream";
            break;
        case DataFormatType::HEX:
            contentType = "text/plain; charset=utf-8";
            break;
        default:
            contentType = "application/octet-stream";
            break;
    }
    
    QByteArray response;
    response += "HTTP/1.1 " + QString::number(statusCode) + " " + statusText + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + QString::number(body.size()) + "\r\n";
    response += "Server: CommLink/1.0\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Connection: keep-alive\r\n";
    response += "\r\n";
    response += body;
    return response;
}

QByteArray HttpServer::buildCORSPreflightResponse() {
    QByteArray response;
    response += "HTTP/1.1 204 No Content\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Accept, Authorization\r\n";
    response += "Access-Control-Max-Age: 86400\r\n";
    response += "Server: CommLink/1.0\r\n";
    response += "\r\n";
    return response;
}
