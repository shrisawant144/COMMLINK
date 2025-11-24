#include "../include/sender.h"
#include "../include/receiverthread.h"
#include <QDebug>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

Sender::~Sender() {
    disconnect();
    stopReceiver();
}

bool Sender::validatePort(quint16 port) const
{
    return port > 0; // quint16 is already limited to 0-65535
}

bool Sender::createSocket(int &fd, int type) const
{
    fd = socket(AF_INET, type, 0);
    if (fd < 0) {
        qCritical() << "Socket creation failed for type" << type;
        return false;
    }
    return true;
}

bool Sender::setupAddress(const QString &host, quint16 port, sockaddr_in &addr) const
{
    if (!validatePort(port)) {
        qCritical() << "Invalid port:" << port;
        return false;
    }
    
    addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    int result = inet_pton(AF_INET, host.toUtf8().constData(), &addr.sin_addr);
    if (result <= 0) {
        qCritical() << "Invalid IP address:" << host;
        return false;
    }
    return true;
}

void Sender::closeSocket(int &fd)
{
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

bool Sender::connectTcp(const QString &host, quint16 port)
{
    disconnect(); // Close existing connection
    
    if (!createSocket(tcp_fd, SOCK_STREAM)) return false;
    
    // Set socket options for better reliability
    int opt = 1;
    setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in addr;
    if (!setupAddress(host, port, addr)) {
        closeSocket(tcp_fd);
        return false;
    }
    
    if (::connect(tcp_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        closeSocket(tcp_fd);
        qCritical() << "TCP connect failed to" << host << ":" << port << "- Error:" << strerror(errno);
        return false;
    }
    
    qInfo() << "✅ TCP connected:" << host << ":" << port;

    sendJson = [this, host, port](const QJsonDocument &doc) {
        if (tcp_fd < 0) {
            qWarning() << "TCP not connected";
            return;
        }
        QByteArray payload = doc.toJson(QJsonDocument::Compact) + "\n";
        ssize_t written = send(tcp_fd, payload.constData(), payload.size(), MSG_NOSIGNAL);
        if (written < 0) {
            qWarning() << "TCP send failed to" << host << ":" << port << "- Error:" << strerror(errno);
            // Try to reconnect on next send
            closeSocket(tcp_fd);
        } else if (written == payload.size()) {
            qInfo() << "→ TCP:" << payload.constData();
        } else {
            qWarning() << "TCP partial send:" << written << "/" << payload.size();
        }
    };
    return true;
}

bool Sender::connectUdp(const QString &host, quint16 port)
{
    disconnect(); // Close existing connection
    
    if (!createSocket(udp_fd, SOCK_DGRAM)) return false;
    
    if (!setupAddress(host, port, udp_addr)) {
        closeSocket(udp_fd);
        return false;
    }

    qInfo() << "✅ UDP configured:" << host << ":" << port;

    sendJson = [this](const QJsonDocument &doc) {
        if (udp_fd < 0) {
            qWarning() << "UDP not configured";
            return;
        }
        QByteArray payload = doc.toJson(QJsonDocument::Compact) + "\n";
        ssize_t written = sendto(udp_fd, payload.constData(), payload.size(), 0,
                               reinterpret_cast<sockaddr*>(&udp_addr), sizeof(udp_addr));
        if (written < 0) {
            qWarning() << "UDP send failed";
        } else if (written == payload.size()) {
            qInfo() << "→ UDP:" << payload.constData();
        } else {
            qWarning() << "UDP partial send:" << written << "/" << payload.size();
        }
    };
    return true;
}

void Sender::disconnect()
{
    closeSocket(tcp_fd);
    closeSocket(udp_fd);
    sendJson = nullptr;
}

Sender::ConnectionType Sender::getConnectionType() const
{
    if (tcp_fd >= 0) return ConnectionType::TCP;
    if (udp_fd >= 0) return ConnectionType::UDP;
    return ConnectionType::TCP; // Default
}

bool Sender::startTcpReceiver(quint16 port)
{
    stopReceiver();
    
    if (!validatePort(port)) {
        qCritical() << "Invalid receiver port:" << port;
        return false;
    }

    if (!createSocket(listen_fd, SOCK_STREAM)) return false;

    // Set socket options for better reliability
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        closeSocket(listen_fd);
        qCritical() << "TCP bind failed on port" << port << "- Error:" << strerror(errno);
        return false;
    }

    if (listen(listen_fd, 5) < 0) {
        closeSocket(listen_fd);
        qCritical() << "TCP listen failed - Error:" << strerror(errno);
        return false;
    }

    receiverThread = new ReceiverThread(listen_fd, true, this);
    if (!receiverThread) {
        closeSocket(listen_fd);
        qCritical() << "Failed to create receiver thread";
        return false;
    }
    
    connect(receiverThread, &ReceiverThread::jsonReceived, this, &Sender::jsonReceived);
    receiverThread->start();

    qInfo() << "✅ TCP Receiver listening on port" << port;
    return true;
}

bool Sender::startUdpReceiver(quint16 port)
{
    stopReceiver();
    
    if (!validatePort(port)) {
        qCritical() << "Invalid receiver port:" << port;
        return false;
    }

    if (!createSocket(listen_fd, SOCK_DGRAM)) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        closeSocket(listen_fd);
        qCritical() << "UDP bind failed on port" << port;
        return false;
    }

    receiverThread = new ReceiverThread(listen_fd, false, this);
    if (!receiverThread) {
        closeSocket(listen_fd);
        qCritical() << "Failed to create receiver thread";
        return false;
    }
    
    connect(receiverThread, &ReceiverThread::jsonReceived, this, &Sender::jsonReceived);
    receiverThread->start();

    qInfo() << "✅ UDP Receiver listening on port" << port;
    return true;
}

void Sender::stopReceiver()
{
    if (receiverThread) {
        receiverThread->stop();
        receiverThread->wait();
        delete receiverThread;
        receiverThread = nullptr;
    }
    closeSocket(listen_fd);
}
