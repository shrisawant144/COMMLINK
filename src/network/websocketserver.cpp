#include "commlink/network/websocketserver.h"
#include <QDateTime>

WebSocketServer::WebSocketServer(QObject *parent)
    : QObject(parent), m_format(DataFormatType::JSON), m_sslEnabled(false) {
    m_server = new QWebSocketServer("CommLink WebSocket Server", 
                                     QWebSocketServer::NonSecureMode, this);
    connect(m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
}

WebSocketServer::~WebSocketServer() {
    stopServer();
}

bool WebSocketServer::startServer(quint16 port) {
    // Close existing server if already listening
    if (m_server->isListening()) {
        m_server->close();
    }
    
    if (m_server->listen(QHostAddress::Any, port)) {
        return true;
    }
    emit errorOccurred(m_server->errorString());
    return false;
}

void WebSocketServer::stopServer() {
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

void WebSocketServer::sendToClient(QWebSocket* client, const DataMessage& message, bool binary) {
    if (!client || !m_clients.contains(client)) return;
    QByteArray data = message.serialize();
    bool success = false;
    if (binary) {
        success = client->sendBinaryMessage(data) >= 0;
    } else {
        success = client->sendTextMessage(QString::fromUtf8(data)) >= 0;
    }
    if (!success) {
        emit errorOccurred("Failed to send message to client: " + client->peerAddress().toString());
    }
}

void WebSocketServer::sendToAll(const DataMessage& message, bool binary) {
    if (m_clients.isEmpty()) {
        emit errorOccurred("No clients connected to broadcast message");
        return;
    }
    
    QByteArray data = message.serialize();
    int successCount = 0;
    
    for (QWebSocket* client : m_clients) {
        if (!client) continue;
        
        bool success = false;
        if (binary) {
            success = client->sendBinaryMessage(data) >= 0;
        } else {
            success = client->sendTextMessage(QString::fromUtf8(data)) >= 0;
        }
        
        if (success) {
            successCount++;
        } else {
            emit errorOccurred("Failed to send broadcast to: " + client->peerAddress().toString());
        }
    }
    
    if (successCount == 0) {
        emit errorOccurred("Failed to send message to any client");
    }
}

QWebSocket* WebSocketServer::findClientByAddress(const QString& addressPort) {
    for (QWebSocket* client : m_clients) {
        QString clientAddr = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
        if (clientAddr == addressPort) {
            return client;
        }
    }
    return nullptr;
}

void WebSocketServer::onNewConnection() {
    if (m_clients.size() >= MAX_CLIENTS) {
        QWebSocket *client = m_server->nextPendingConnection();
        client->close();
        client->deleteLater();
        emit errorOccurred("Max client limit reached. Connection refused.");
        return;
    }
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
