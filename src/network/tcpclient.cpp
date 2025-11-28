#include "commlink/network/tcpclient.h"
#include <QDateTime>

TcpClient::TcpClient(QObject *parent) 
    : QObject(parent), m_format(DataFormatType::JSON) {
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), 
            this, &TcpClient::onError);
}

bool TcpClient::connectToHost(const QString& host, quint16 port) {
    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected(3000);
}

void TcpClient::disconnect() {
    m_socket->disconnectFromHost();
}

void TcpClient::sendMessage(const DataMessage& message) {
    QByteArray data = message.serialize();
    m_socket->write(data);
    m_socket->flush();
}

bool TcpClient::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpClient::onConnected() {
    emit connected();
}

void TcpClient::onDisconnected() {
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
    emit errorOccurred(m_socket->errorString());
}
