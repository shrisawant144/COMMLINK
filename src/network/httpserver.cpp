#include "commlink/network/httpserver.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTimer>

HttpServer::HttpServer(QObject *parent)
    : QObject(parent), m_format(DataFormatType::JSON), m_sslEnabled(false) {
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &HttpServer::onNewConnection);
}

bool HttpServer::startServer(quint16 port, const QHostAddress& bindAddress) {
    // Close existing server if already listening
    if (m_server->isListening()) {
        m_server->close();
    }
    
    // Set max connections
    m_server->setMaxPendingConnections(MAX_CLIENTS);
    
    if (m_server->listen(bindAddress, port)) {
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
    m_requestBuffers.clear();
    m_messageQueue.clear();
    queuedBytes = 0;
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
    auto* idle = new QTimer(socket);
    idle->setSingleShot(true);
    idle->setInterval(30000);
    idle->start();
    connect(socket, &QTcpSocket::readyRead, idle, QOverload<>::of(&QTimer::start));
    connect(idle, &QTimer::timeout, socket, &QTcpSocket::disconnectFromHost);
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
    if (m_requestBuffers[socket].size() > 8 * 1024 * 1024 + 65536) {
        socket->write(buildResponse(413, "Request too large", DataFormatType::TEXT));
        socket->disconnectFromHost();
        return;
    }
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
    for (const auto &message : m_messageQueue.value(socket)) queuedBytes -= message.serialize().size();
    m_messageQueue.remove(socket);
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
            return QJsonDocument(QJsonObject{{"status", "received"},
                                             {"method", request.method},
                                             {"path", request.path}})
                .toJson(QJsonDocument::Compact);
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
    auto& buffer = m_requestBuffers[socket];
    auto reject = [this, socket](int status, const QByteArray& reason) {
        emit errorOccurred(QString::fromLatin1(reason));
        socket->write(buildResponse(status, reason, DataFormatType::TEXT));
        socket->disconnectFromHost();
        return false;
    };
    const int headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd < 0)
        return buffer.size() > 65536 ? reject(431, "Headers too large") : false;
    if (headerEnd > 65536)
        return reject(431, "Headers too large");
    const auto lines = buffer.left(headerEnd).split('\n');
    const auto first = lines.first().trimmed().split(' ');
    if (first.size() != 3 || !first[2].startsWith("HTTP/1."))
        return reject(400, "Malformed HTTP request");
    HttpRequest request;
    request.method = QString::fromLatin1(first[0]);
    request.path = QString::fromUtf8(first[1]);
    for (int i = 1; i < lines.size(); ++i) {
        const auto line = lines[i].trimmed();
        const int colon = line.indexOf(':');
        if (colon <= 0)
            return reject(400, "Malformed HTTP header");
        const auto name = QString::fromLatin1(line.left(colon)).toLower();
        const auto value = QString::fromLatin1(line.mid(colon + 1)).trimmed();
        if (request.headers.contains(name) && name == "content-length")
            return reject(400, "Duplicate content length");
        request.headers[name] = value;
    }
    if (request.headers.contains("transfer-encoding"))
        return reject(501, "Transfer encoding is not supported by this mock server");
    bool lengthOK = true;
    const qint64 length = request.headers.contains("content-length")
                              ? request.headers["content-length"].toLongLong(&lengthOK)
                              : 0;
    if (!lengthOK || length < 0)
        return reject(400, "Invalid content length");
    if (length > 8 * 1024 * 1024)
        return reject(413, "Body too large");
    const int bodyStart = headerEnd + 4;
    if (buffer.size() < bodyStart + length)
        return false;
    request.body = buffer.mid(bodyStart, int(length));
    buffer.remove(0, bodyStart + int(length));
    if (request.method == "OPTIONS") {
        socket->write(buildCORSPreflightResponse());
        return true;
    }
    const auto format = request.headers.contains("content-type")
                            ? detectContentType(request.headers["content-type"])
                            : m_format;
    emit messageReceived(DataMessage::deserialize(request.body, format),
                         m_clients[socket] + " [" + request.method + " " + request.path + "]",
                         QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    auto responseFormat =
        request.headers.contains("accept") ? detectAcceptType(request.headers["accept"]) : format;
    QByteArray body;
    if (request.method != "HEAD" && !m_messageQueue[socket].isEmpty()) {
        const auto message = m_messageQueue[socket].takeFirst();
        body = message.serialize();
        queuedBytes -= body.size();
        responseFormat = message.type;
    } else
        body = buildResponseBody(request, responseFormat);
    auto response = buildResponse(200, body, responseFormat);
    if (request.method == "HEAD" && !body.isEmpty())
        response.chop(body.size());
    socket->write(response);
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
    response += ("HTTP/1.1 " + QString::number(statusCode) + " " + statusText + "\r\n").toUtf8();
    response += ("Content-Type: " + contentType + "\r\n").toUtf8();
    response += ("Content-Length: " + QString::number(body.size()) + "\r\n").toUtf8();
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

void HttpServer::sendToAll(const DataMessage& message) {
    queueMessageForAll(message);
}
void HttpServer::sendToClient(QTcpSocket* client, const DataMessage& message) {
    if (!m_clients.contains(client)) {
        emit errorOccurred("Client is not connected");
        return;
    }
    queueMessageForClient(client, message);
}

QTcpSocket* HttpServer::findClientByAddress(const QString& addressPort) {
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value() == addressPort) {
            return it.key();
        }
    }
    return nullptr;
}

void HttpServer::queueMessageForClient(QTcpSocket *client, const DataMessage &message) {
    const int bytes = message.serialize().size();
    if (!m_clients.contains(client)) { emit errorOccurred("HTTP client is not connected"); return; }
    if (m_messageQueue[client].size() >= 100 || bytes > 8 * 1024 * 1024 || queuedBytes + bytes > 32 * 1024 * 1024) {
        emit errorOccurred("HTTP outgoing queue limit reached"); return;
    }
    m_messageQueue[client].append(message); queuedBytes += bytes;
}
void HttpServer::queueMessageForAll(const DataMessage &message) {
    for (auto *client : m_clients.keys()) queueMessageForClient(client, message);
}

bool HttpServer::hasQueuedMessages(QTcpSocket* client) const {
    return m_messageQueue.contains(client) && !m_messageQueue[client].isEmpty();
}
