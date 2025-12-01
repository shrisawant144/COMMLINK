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
    m_socket->connectToHost(host, port);
    m_connectionTimer->start(CONNECTION_TIMEOUT_MS);
}

void TcpClient::disconnect() {
    m_connectionTimer->stop();
    m_connected = false;
    m_socket->disconnectFromHost();
}

void TcpClient::sendMessage(const DataMessage& message) {
    QByteArray data = message.serialize();
    qint64 bytesWritten = m_socket->write(data);
    if (bytesWritten == -1) {
        emit errorOccurred("Failed to write data: " + m_socket->errorString());
        return;
    }
    if (!m_socket->flush()) {
        emit errorOccurred("Failed to flush socket");
    }
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
    QByteArray data = m_socket->readAll();
    DataMessage msg = DataMessage::deserialize(data, m_format);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString source = m_socket->peerAddress().toString() + ":" + QString::number(m_socket->peerPort());
    emit messageReceived(msg, source, timestamp);
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
