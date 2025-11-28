#include "commlink/network/websocketserver.h"
#include <QDateTime>

WebSocketServer::WebSocketServer(QObject *parent)
    : QObject(parent), m_format(DataFormatType::JSON) {
    m_server = new QWebSocketServer("CommLink WebSocket Server", 
                                     QWebSocketServer::NonSecureMode, this);
    connect(m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
}

WebSocketServer::~WebSocketServer() {
    close();
}

bool WebSocketServer::listen(quint16 port) {
    if (m_server->listen(QHostAddress::Any, port)) {
        return true;
    }
    emit errorOccurred(m_server->errorString());
    return false;
}

void WebSocketServer::close() {
    for (QWebSocket *client : m_clients) {
        client->close();
        client->deleteLater();
    }
    m_clients.clear();
    m_server->close();
}

bool WebSocketServer::isListening() const {
    return m_server->isListening();
}

void WebSocketServer::onNewConnection() {
    QWebSocket *client = m_server->nextPendingConnection();
    
    connect(client, &QWebSocket::textMessageReceived, this, &WebSocketServer::onTextMessageReceived);
    connect(client, &QWebSocket::binaryMessageReceived, this, &WebSocketServer::onBinaryMessageReceived);
    connect(client, &QWebSocket::disconnected, this, &WebSocketServer::onClientDisconnected);
    
    m_clients.append(client);
    
    QString clientInfo = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
    emit clientConnected(clientInfo);
}

void WebSocketServer::onTextMessageReceived(const QString& message) {
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    
    DataMessage msg = DataMessage::deserialize(message.toUtf8(), m_format);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString source = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
    
    emit messageReceived(msg, source, timestamp);
}

void WebSocketServer::onBinaryMessageReceived(const QByteArray& message) {
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    
    DataMessage msg = DataMessage::deserialize(message, DataFormatType::BINARY);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString source = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
    
    emit messageReceived(msg, source, timestamp);
}

void WebSocketServer::onClientDisconnected() {
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    
    QString clientInfo = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
    m_clients.removeAll(client);
    client->deleteLater();
    
    emit clientDisconnected(clientInfo);
}
