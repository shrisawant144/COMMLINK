#include "commlink/network/udpclient.h"
#include <QDateTime>

UdpClient::UdpClient(QObject *parent) 
    : QObject(parent), m_port(0), m_connected(false), m_format(DataFormatType::JSON) {
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::connected, this, [this] {
        m_connected = true;
        emit connected();
    });
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QUdpSocket::error), this,
            [this](QAbstractSocket::SocketError) {
                m_connected = false;
                emit errorOccurred(m_socket->errorString());
            });
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpClient::onReadyRead);
}

bool UdpClient::connectToHost(const QString& host, quint16 port) {
    m_socket->abort();
    m_connected = false;
    m_port = port;
    if (host.trimmed().isEmpty() || !port) {
        emit errorOccurred("Invalid UDP endpoint");
        return false;
    }
    m_socket->connectToHost(host, port);
    return true;
}

void UdpClient::disconnect() {
    m_socket->close();
    m_connected = false;
    emit disconnected();
}

void UdpClient::sendMessage(const DataMessage& message) {
    if (!m_connected) {
        emit errorOccurred("UDP endpoint is not ready");
        return;
    }
    QByteArray data = message.serialize();
    qint64 bytesWritten = m_socket->write(data);
    if (bytesWritten == -1) {
        emit errorOccurred("Failed to send datagram: " + m_socket->errorString());
    }
}

void UdpClient::onReadyRead() {
    while (m_socket->hasPendingDatagrams()) {
        QByteArray buffer;
        qint64 datagramSize = m_socket->pendingDatagramSize();
        buffer.resize(static_cast<int>(datagramSize));
        QHostAddress sender;
        quint16 senderPort = 0;
        
        m_socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
        
        DataMessage msg = DataMessage::deserialize(buffer, m_format);
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString source = sender.toString() + ":" + QString::number(senderPort);
        
        emit messageReceived(msg, source, timestamp);
    }
}
