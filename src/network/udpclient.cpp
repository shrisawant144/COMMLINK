#include "commlink/network/udpclient.h"
#include <QDateTime>

UdpClient::UdpClient(QObject *parent) 
    : QObject(parent), m_port(0), m_connected(false), m_format(DataFormatType::JSON) {
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpClient::onReadyRead);
}

bool UdpClient::connectToHost(const QString& host, quint16 port) {
    m_host = QHostAddress(host);
    m_port = port;
    
    if (m_socket->bind()) {
        m_connected = true;
        emit connected();
        return true;
    }
    emit errorOccurred("Failed to bind UDP socket");
    return false;
}

void UdpClient::disconnect() {
    m_socket->close();
    m_connected = false;
    emit disconnected();
}

void UdpClient::sendMessage(const DataMessage& message) {
    QByteArray data = message.serialize();
    m_socket->writeDatagram(data, m_host, m_port);
}

void UdpClient::onReadyRead() {
    while (m_socket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(m_socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        
        m_socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
        
        DataMessage msg = DataMessage::deserialize(buffer, m_format);
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString source = sender.toString() + ":" + QString::number(senderPort);
        
        emit messageReceived(msg, source, timestamp);
    }
}
