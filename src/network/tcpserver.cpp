#include "commlink/network/tcpserver.h"

#include <QDateTime>
#include <QPointer>

TcpServer::TcpServer(QObject *parent)
    : QObject(parent), m_format(DataFormatType::JSON), m_sslEnabled(false), m_idleTimeout(300) {
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
    
    m_idleTimer = new QTimer(this);
    connect(m_idleTimer, &QTimer::timeout, this, &TcpServer::checkIdleConnections);
    m_idleTimer->start(60000); // Check every minute
}

TcpServer::~TcpServer() {
    stopServer();
}

bool TcpServer::startServer(quint16 port, const QHostAddress& bindAddress) {
    // Close any existing server first
    if (m_server->isListening()) {
        m_server->close();
    }
    
    // Set socket options to allow address reuse
    m_server->setMaxPendingConnections(MAX_CLIENTS);
    
    if (m_server->listen(bindAddress, port)) {
        return true;
    }
    emit errorOccurred(m_server->errorString());
    return false;
}

void TcpServer::stopServer() {
    for (QTcpSocket* client : QList<QTcpSocket*>(m_clients)) {
        client->disconnectFromHost();
        client->deleteLater();
    }
    m_clients.clear();
    m_lastActivity.clear();
    m_framers.clear();
    peerEndpoints.clear();
    scheduledReads.clear();
    m_server->close();
}

bool TcpServer::isListening() const {
    return m_server->isListening();
}


void TcpServer::sendToAll(const DataMessage& message) {
    if (message.serialize().size() > StreamFramer::MaximumPayload) {
        emit errorOccurred("TCP payload exceeds 8 MiB");
        return;
    }
    QByteArray data = m_framing.encode(message.serialize());
    for (QTcpSocket* client : QList<QTcpSocket*>(m_clients)) {
        if (client->bytesToWrite() + data.size() > 16 * 1024 * 1024) {
            emit errorOccurred("TCP client send buffer is full");
            continue;
        }
        qint64 bytesWritten = client->write(data);
        if (bytesWritten == -1) {
            emit errorOccurred("Failed to write data to client: " + client->peerAddress().toString());
            continue;
        }
        client->flush();
    }
}

void TcpServer::sendToClient(QTcpSocket* client, const DataMessage& message) {
    if (!client || !m_clients.contains(client)) return;
    if (message.serialize().size() > StreamFramer::MaximumPayload) {
        emit errorOccurred("TCP payload exceeds 8 MiB");
        return;
    }
    QByteArray data = m_framing.encode(message.serialize());
    if (client->bytesToWrite() + data.size() > 16 * 1024 * 1024) {
        emit errorOccurred("TCP client send buffer is full");
        return;
    }
    qint64 bytesWritten = client->write(data);
    if (bytesWritten == -1) {
        emit errorOccurred("Failed to write data to client: " + client->peerAddress().toString());
        return;
    }
    client->flush();
}

QTcpSocket* TcpServer::findClientByAddress(const QString& addressPort) {
    for (QTcpSocket* client : m_clients) {
        QString clientAddr = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
        if (clientAddr == addressPort) {
            return client;
        }
    }
    return nullptr;
}

void TcpServer::onNewConnection() {
    if (m_clients.size() >= MAX_CLIENTS) {
        QTcpSocket *client = m_server->nextPendingConnection();
        client->disconnectFromHost();
        client->deleteLater();
        emit errorOccurred("Max client limit reached. Connection refused.");
        return;
    }
    QTcpSocket *client = m_server->nextPendingConnection();
    connect(client, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnected);
    client->setReadBufferSize(StreamFramer::MaximumPayload + 32);
    m_clients.append(client);
    m_framers.insert(client, m_framing);
    client->setSocketOption(QAbstractSocket::KeepAliveOption, m_keepAlive ? 1 : 0);
    m_lastActivity[client] = QDateTime::currentSecsSinceEpoch();
    QString clientInfo = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
    peerEndpoints[client] = clientInfo;
    emit clientConnected(clientInfo);
}

void TcpServer::onReadyRead() {
    auto* client = qobject_cast<QTcpSocket*>(sender());
    if (client && readBatch(client))
        scheduleRead(client);
}
void TcpServer::scheduleRead(QTcpSocket* client) {
    if (scheduledReads.contains(client))
        return;
    scheduledReads.insert(client);
    QPointer<QTcpSocket> guard(client);
    QTimer::singleShot(0, this, [this, guard] {
        if (!guard)
            return;
        scheduledReads.remove(guard);
        if (readBatch(guard))
            scheduleRead(guard);
    });
}
bool TcpServer::readBatch(QTcpSocket* client) {
    if (!m_framers.contains(client))
        return false;
    m_lastActivity[client] = QDateTime::currentSecsSinceEpoch();
    auto& framer = m_framers[client];
    const auto frames = framer.feed(client->bytesAvailable() > 0
                                        ? client->read(framer.mode == StreamFramer::Raw
                                                           ? StreamFramer::MaximumPayload
                                                           : StreamFramer::MaximumPayload + 32)
                                        : QByteArray());
    const auto error = framer.error;
    const auto endpoint = peerEndpoints.value(client);
    for (const auto& frame : frames) {
        if (!m_framers.contains(client))
            return false;
        auto message = DataMessage::deserialize(frame.payload, m_format);
        message.wireBytes = frame.wire;
        emit messageReceived(message, endpoint,
                             QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    }
    if (!error.isEmpty()) {
        emit errorOccurred(error);
        client->abort();
        return false;
    }
    return m_framers.contains(client) &&
           (frames.size() == StreamFramer::FrameBatchSize || client->bytesAvailable() > 0);
}

void TcpServer::onClientDisconnected() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    const QString clientInfo = peerEndpoints.value(client);
    while (readBatch(client)) {
    }
    m_clients.removeAll(client);
    m_lastActivity.remove(client);
    m_framers.remove(client);
    peerEndpoints.remove(client);
    scheduledReads.remove(client);

    emit clientDisconnected(clientInfo);
    client->deleteLater();
}

void TcpServer::checkIdleConnections() {
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    QList<QTcpSocket*> toDisconnect;
    
    for (auto it = m_lastActivity.begin(); it != m_lastActivity.end(); ++it) {
        if (currentTime - it.value() > m_idleTimeout) {
            toDisconnect.append(it.key());
        }
    }
    
    for (QTcpSocket *client : toDisconnect) {
        QString clientInfo = client->peerAddress().toString() + ":" + QString::number(client->peerPort());
        emit errorOccurred("Idle timeout: disconnecting client " + clientInfo);
        client->disconnectFromHost();
    }
}

void TcpServer::setFraming(StreamFramer::Mode mode, const QByteArray& delimiter) {
    if (m_framing.mode == mode && m_framing.delimiter == delimiter)
        return;
    m_framing.reset();
    m_framing.mode = mode;
    m_framing.delimiter = delimiter;
    for (auto& framer : m_framers)
        framer = m_framing;
}
void TcpServer::setKeepAlive(bool enabled) {
    m_keepAlive = enabled;
    for (auto* client : m_clients)
        client->setSocketOption(QAbstractSocket::KeepAliveOption, enabled ? 1 : 0);
}
quint16 TcpServer::localPort() const {
    return m_server->serverPort();
}
