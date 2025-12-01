#include "commlink/network/websocketclient.h"
#include <QDateTime>

WebSocketClient::WebSocketClient(QObject *parent) 
    : QObject(parent), m_format(DataFormatType::JSON), m_connected(false) {
    connect(&m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
    connect(&m_socket, &QWebSocket::binaryMessageReceived, this, &WebSocketClient::onBinaryMessageReceived);
    connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), 
            this, &WebSocketClient::onError);
}

void WebSocketClient::connectToServer(const QString& url) {
    m_socket.open(QUrl(url));
}

void WebSocketClient::disconnect() {
    m_connected = false;
    m_socket.close();
}

void WebSocketClient::sendMessage(const DataMessage& message) {
    QByteArray data = message.serialize();
    qint64 bytesSent = 0;
    if (m_format == DataFormatType::BINARY) {
        bytesSent = m_socket.sendBinaryMessage(data);
    } else {
        bytesSent = m_socket.sendTextMessage(QString::fromUtf8(data));
    }
    if (bytesSent == 0) {
        emit errorOccurred("Failed to send WebSocket message");
    }
}

bool WebSocketClient::isConnected() const {
    return m_connected;
}

void WebSocketClient::onConnected() {
    m_connected = true;
    emit connected();
}

void WebSocketClient::onDisconnected() {
    m_connected = false;
    emit disconnected();
}

void WebSocketClient::onTextMessageReceived(const QString& message) {
    DataMessage msg = DataMessage::deserialize(message.toUtf8(), m_format);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    emit messageReceived(msg, m_socket.peerAddress().toString(), timestamp);
}

void WebSocketClient::onBinaryMessageReceived(const QByteArray& message) {
    DataMessage msg = DataMessage::deserialize(message, DataFormatType::BINARY);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    emit messageReceived(msg, m_socket.peerAddress().toString(), timestamp);
}

void WebSocketClient::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    emit errorOccurred(m_socket.errorString());
}
