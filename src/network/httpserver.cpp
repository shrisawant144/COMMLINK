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
    HttpRequest request = parseRequest(data);
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString source = m_clients[socket] + " [" + request.method + " " + request.path + "]";
    
    DataMessage msg = DataMessage::deserialize(request.body, m_format);
    emit messageReceived(msg, source, timestamp);
    
    // Send response
    QString responseBody = "{\"status\":\"received\",\"method\":\"" + request.method + "\",\"path\":\"" + request.path + "\"}";
    QByteArray response = buildResponse(200, responseBody);
    socket->write(response);
    socket->flush();
}

void HttpServer::onClientDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString clientInfo = m_clients.take(socket);
    emit clientDisconnected(clientInfo);
    socket->deleteLater();
}

HttpServer::HttpRequest HttpServer::parseRequest(const QByteArray& data) {
    HttpRequest request;
    QString dataStr = QString::fromUtf8(data);
    QStringList lines = dataStr.split("\r\n");
    
    if (!lines.isEmpty()) {
        QStringList requestLine = lines[0].split(" ");
        if (requestLine.size() >= 2) {
            request.method = requestLine[0];
            request.path = requestLine[1];
        }
    }
    
    int bodyStart = dataStr.indexOf("\r\n\r\n");
    if (bodyStart != -1) {
        request.body = data.mid(bodyStart + 4);
    }
    
    return request;
}

QByteArray HttpServer::buildResponse(int statusCode, const QString& body) {
    QString statusText = (statusCode == 200) ? "OK" : "Error";
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
