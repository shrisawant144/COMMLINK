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
    
    DataMessage msg = DataMessage::deserialize(request.body, m_format);
    emit messageReceived(msg, source, timestamp);
    
    // Send response
    QString responseBody = "{\"status\":\"received\",\"method\":\"" + request.method + "\",\"path\":\"" + request.path + "\"}";
    QByteArray response = buildResponse(200, responseBody);
    socket->write(response);
    socket->flush();
    
    return true;
}

QByteArray HttpServer::buildResponse(int statusCode, const QString& body) {
    static const int HTTP_OK = 200;
    QString statusText = (statusCode == HTTP_OK) ? "OK" : "Error";
    QByteArray response;
    response += "HTTP/1.1 " + QString::number(statusCode) + " " + statusText + "\r\n";
    response += "Content-Type: application/json\r\n";
    response += "Content-Length: " + QString::number(body.toUtf8().size()) + "\r\n";
    response += "Server: CommLink/1.0\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "\r\n";
    response += body.toUtf8();
    return response;
}
