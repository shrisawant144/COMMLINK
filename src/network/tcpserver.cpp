#include "commlink/network/tcpserver.h"
#include <QDateTime>

TcpServer::TcpServer(QObject *parent)
    : QObject(parent), m_format(DataFormatType::JSON) {
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
}

TcpServer::~TcpServer() {
    close();
}

bool TcpServer::listen(quint16 port) {
    if (m_server->listen(QHostAddress::Any, port)) {
        return true;
    }
    emit errorOccurred(m_server->errorString());
    return false;
}

void TcpServer::close() {
    for (QTcpSocket *client : m_clients) {
        client->disconnectFromHost();
        client->deleteLater();
    }
    m_clients.clear();
    m_server->close();
}

bool TcpServer::isListening() const {
    return m_server->isListening();
}

void TcpServer::sendToAll(const DataMessage& message) {
    QByteArray data = message.serialize();
    for (QTcpSocket *client : m_clients) {
        client->write(data);
        client->flush();
    }
}

void TcpServer::onNewConnection() {
    QTcpSocket *client = m_server->nextPendingConnection();
    
    connect(client, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnected);
    
    m_clients.append(client);
    
    QString clientInfo = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
    emit clientConnected(clientInfo);
}

void TcpServer::onReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    QByteArray data = client->readAll();
    DataMessage msg = DataMessage::deserialize(data, m_format);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString source = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
    
    emit messageReceived(msg, source, timestamp);
}

void TcpServer::onClientDisconnected() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    QString clientInfo = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
    m_clients.removeAll(client);
    client->deleteLater();
    
    emit clientDisconnected(clientInfo);
}
