#include "commlink/network/udpserver.h"
#include <QDateTime>

UdpServer::UdpServer(QObject *parent)
    : QObject(parent), m_listening(false), m_format(DataFormatType::JSON) {
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpServer::onReadyRead);
}

bool UdpServer::startServer(quint16 port) {
    if (m_socket->bind(QHostAddress::Any, port)) {
        m_listening = true;
        return true;
    }
    emit errorOccurred(m_socket->errorString());
    return false;
}

void UdpServer::stopServer() {
    m_socket->close();
    m_listening = false;
}


void UdpServer::sendTo(const QHostAddress& address, quint16 port, const DataMessage& message) {
    QByteArray data = message.serialize();
    qint64 bytesWritten = m_socket->writeDatagram(data, address, port);
    if (bytesWritten == -1) {
        emit errorOccurred("Failed to send datagram to " + address.toString() + ":" + QString::number(port));
    }
}

void UdpServer::onReadyRead() {
    while (m_socket->hasPendingDatagrams()) {
        qint64 datagramSize = m_socket->pendingDatagramSize();
        if (datagramSize > MAX_BUFFER_SIZE) {
            emit errorOccurred("Buffer overflow: received datagram exceeds max buffer size.");
            m_socket->readDatagram(nullptr, 0); // drop datagram
            continue;
        }
        int bufferSize = static_cast<int>(datagramSize);
        QByteArray buffer;
        buffer.resize(bufferSize);
        QHostAddress sender;
        quint16 senderPort = 0;
        m_socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
        DataMessage msg = DataMessage::deserialize(buffer, m_format);
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString source = sender.toString() + ":" + QString::number(senderPort);
        emit messageReceived(msg, source, timestamp);
    }
}
