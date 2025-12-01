#include "commlink/network/httpserver.h"
#include <QDateTime>
#include <QRegularExpression>

HttpServer::HttpServer(QObject *parent)
    : QObject(parent), m_format(DataFormatType::JSON) {
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
    }
    m_clients.clear();
    m_server->close();
}

bool HttpServer::isListening() const {
    return m_server->isListening();
}

void HttpServer::onNewConnection() {
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
        // Remove processed request from buffer
        return true;
    }
    
    // Detect format from Content-Type header if available
    DataFormatType requestFormat = m_format;
    if (request.headers.contains("Content-Type")) {
        QString contentType = request.headers["Content-Type"].toLower();
        // Remove charset and other parameters
        contentType = contentType.split(';').first().trimmed();
        
        if (contentType == "application/json") {
            requestFormat = DataFormatType::JSON;
        } else if (contentType == "application/xml" || contentType == "text/xml") {
            requestFormat = DataFormatType::XML;
        } else if (contentType == "text/csv") {
            requestFormat = DataFormatType::CSV;
        } else if (contentType == "text/plain") {
            requestFormat = DataFormatType::TEXT;
        } else if (contentType == "application/octet-stream") {
            requestFormat = DataFormatType::BINARY;
        }
    }
    
    DataMessage msg = DataMessage::deserialize(request.body, requestFormat);
    emit messageReceived(msg, source, timestamp);
    
    // Create response in the same format as the request (or use Accept header if provided)
    DataFormatType responseFormat = requestFormat;
    if (request.headers.contains("Accept")) {
        QString accept = request.headers["Accept"].toLower();
        accept = accept.split(';').first().split(',').first().trimmed();
        
        if (accept == "application/json") {
            responseFormat = DataFormatType::JSON;
        } else if (accept == "application/xml" || accept == "text/xml") {
            responseFormat = DataFormatType::XML;
        } else if (accept == "text/csv") {
            responseFormat = DataFormatType::CSV;
        } else if (accept == "text/plain") {
            responseFormat = DataFormatType::TEXT;
        } else if (accept == "application/octet-stream") {
            responseFormat = DataFormatType::BINARY;
        } else if (accept != "*/*" && !accept.isEmpty()) {
            // If a specific Accept header is provided but not recognized, still use it
            // This allows for future extensibility
        }
    }
    
    // Build response message based on format
    QByteArray responseBody;
    switch (responseFormat) {
        case DataFormatType::JSON: {
            QString jsonResponse = R"({"status":"received","method":")" + request.method + R"(","path":")" + request.path + R"("})";
            responseBody = jsonResponse.toUtf8();
            break;
        }
        case DataFormatType::XML: {
            QString xmlResponse = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<response>\n  <status>received</status>\n  <method>" + 
                                 request.method + "</method>\n  <path>" + request.path + "</path>\n</response>";
            responseBody = xmlResponse.toUtf8();
            break;
        }
        case DataFormatType::CSV: {
            QString csvResponse = "status,method,path\nreceived," + request.method + "," + request.path;
            responseBody = csvResponse.toUtf8();
            break;
        }
        case DataFormatType::TEXT: {
            QString textResponse = "Status: received\nMethod: " + request.method + "\nPath: " + request.path;
            responseBody = textResponse.toUtf8();
            break;
        }
        case DataFormatType::BINARY:
        case DataFormatType::HEX:
        default: {
            // For binary/hex or unknown formats, echo back the received data
            responseBody = request.body;
            break;
        }
    }
    
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
