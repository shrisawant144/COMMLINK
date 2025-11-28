#include "commlink/network/udpserver.h"
#include <QDateTime>

UdpServer::UdpServer(QObject *parent)
    : QObject(parent), m_listening(false), m_format(DataFormatType::JSON) {
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpServer::onReadyRead);
}

bool UdpServer::listen(quint16 port) {
    if (m_socket->bind(QHostAddress::Any, port)) {
        m_listening = true;
        return true;
    }
    emit errorOccurred(m_socket->errorString());
    return false;
}

void UdpServer::close() {
    m_socket->close();
    m_listening = false;
}

void UdpServer::onReadyRead() {
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
