#include "commlink/network/websocketclient.h"
#include <QDateTime>

WebSocketClient::WebSocketClient(QObject *parent) 
    : QObject(parent), m_format(DataFormatType::JSON) {
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
    m_socket.close();
}

void WebSocketClient::sendMessage(const DataMessage& message) {
    QByteArray data = message.serialize();
    if (m_format == DataFormatType::BINARY) {
        m_socket.sendBinaryMessage(data);
    } else {
        m_socket.sendTextMessage(QString::fromUtf8(data));
    }
}

bool WebSocketClient::isConnected() const {
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

void WebSocketClient::onConnected() {
    emit connected();
}

void WebSocketClient::onDisconnected() {
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
