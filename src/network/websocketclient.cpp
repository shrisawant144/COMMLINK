#include "commlink/network/websocketclient.h"
#include <QDateTime>

WebSocketClient::WebSocketClient(QObject *parent) 
    : QObject(parent), m_format(DataFormatType::JSON), m_connected(false) {
    m_socket.setMaxAllowedIncomingMessageSize(8 * 1024 * 1024);
    connect(&pingTimer, &QTimer::timeout, this, [this] {
        if (m_connected)
            m_socket.ping("CommLink");
    });
    connect(&m_socket, &QWebSocket::pong, this, [this](quint64 elapsed, const QByteArray& payload) {
        emit diagnostic(QString("Pong: %1 ms; payload %2")
                            .arg(elapsed)
                            .arg(QString::fromLatin1(payload.toHex())));
    });
    connect(&m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
    connect(&m_socket, &QWebSocket::binaryMessageReceived, this, &WebSocketClient::onBinaryMessageReceived);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(&m_socket, &QWebSocket::errorOccurred,
#else
    connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
#endif
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
    if (!m_connected || m_socket.bytesToWrite() + data.size() > 16 * 1024 * 1024) { emit errorOccurred("WebSocket is disconnected or its send buffer is full"); return; }
    qint64 bytesSent = 0;
    if (message.type == DataFormatType::BINARY || message.type == DataFormatType::HEX) {
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
    emit diagnostic(QString("WebSocket closed: code %1; %2")
                        .arg(int(m_socket.closeCode()))
                        .arg(m_socket.closeReason()));
    emit disconnected();
}

void WebSocketClient::onTextMessageReceived(const QString& message) {
    DataMessage msg = DataMessage::deserialize(message.toUtf8(), m_format);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    emit messageReceived(
        msg, m_socket.peerAddress().toString() + ":" + QString::number(m_socket.peerPort()),
        timestamp);
}

void WebSocketClient::onBinaryMessageReceived(const QByteArray& message) {
    DataMessage msg = DataMessage::deserialize(message, DataFormatType::BINARY);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    emit messageReceived(
        msg, m_socket.peerAddress().toString() + ":" + QString::number(m_socket.peerPort()),
        timestamp);
}

void WebSocketClient::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    emit errorOccurred(m_socket.errorString());
}

void WebSocketClient::setPingInterval(int milliseconds) {
    if (milliseconds <= 0)
        pingTimer.stop();
    else
        pingTimer.start(qMax(100, milliseconds));
}
void WebSocketClient::ping(const QByteArray& payload) {
    if (m_connected)
        m_socket.ping(payload.left(125));
}
