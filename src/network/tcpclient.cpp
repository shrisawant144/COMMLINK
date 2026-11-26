#include "commlink/network/tcpclient.h"
#include <QDateTime>

TcpClient::TcpClient(QObject *parent) 
    : QObject(parent), m_format(DataFormatType::JSON), m_connected(false) {
    m_socket = new QTcpSocket(this);
    m_connectionTimer = new QTimer(this);
    m_connectionTimer->setSingleShot(true);
    
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), 
            this, &TcpClient::onError);
    connect(m_connectionTimer, &QTimer::timeout, this, &TcpClient::onConnectionTimeout);
}

void TcpClient::connectToHost(const QString& host, quint16 port) {
    m_socket->abort();
    m_framer.reset();
    m_socket->connectToHost(host, port);
    m_connectionTimer->start(CONNECTION_TIMEOUT_MS);
}

void TcpClient::disconnect() {
    m_connectionTimer->stop();
    m_connected = false;
    m_socket->disconnectFromHost();
}

void TcpClient::sendMessage(const DataMessage& message) {
    QByteArray data = m_framer.encode(message.serialize());
    if (!m_connected || message.serialize().size() > StreamFramer::MaximumPayload || (m_framer.mode == StreamFramer::Delimiter && m_framer.delimiter.isEmpty())) {
        emit errorOccurred("TCP client is not connected or payload is too large"); return;
    }
    qint64 bytesWritten = m_socket->write(data);
    if (bytesWritten == -1) {
        emit errorOccurred("Failed to write data: " + m_socket->errorString());
        return;
    }
    m_socket->flush();
}

bool TcpClient::isConnected() const {
    return m_connected;
}

void TcpClient::onConnected() {
    m_connectionTimer->stop();
    m_connected = true;
    emit connected();
}

void TcpClient::onDisconnected() {
    m_connectionTimer->stop();
    m_connected = false;
    emit disconnected();
}

void TcpClient::onReadyRead() {
    const auto frames = m_framer.feed(m_socket->readAll());
    for (const auto &frame : frames) {
        DataMessage msg = DataMessage::deserialize(frame.payload, m_format);
        msg.wireBytes = frame.wire;
        emit messageReceived(msg, m_socket->peerAddress().toString() + ":" + QString::number(m_socket->peerPort()),
                             QDateTime::currentDateTime().toString(Qt::ISODateWithMs));
    }
    if (!m_framer.error.isEmpty()) { emit errorOccurred(m_framer.error); m_socket->abort(); }
}

void TcpClient::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    m_connectionTimer->stop();
    emit errorOccurred(m_socket->errorString());
}

void TcpClient::onConnectionTimeout() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->abort();
        emit errorOccurred("Connection timeout");
    }
}

void TcpClient::setFraming(StreamFramer::Mode mode, const QByteArray &delimiter) {
    m_framer.reset(); m_framer.mode = mode; m_framer.delimiter = delimiter;
}
void TcpClient::setKeepAlive(bool enabled) { m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, enabled ? 1 : 0); }
quint16 TcpClient::localPort() const { return m_socket->localPort(); }
