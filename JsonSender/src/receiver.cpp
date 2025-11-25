#include "../include/receiver.h"
#include "../include/receiverthread.h"
#include <QtCore/QDebug>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

Receiver::~Receiver() {
    disconnect();
}

bool Receiver::validatePort(quint16 port) const
{
    return port > 0; // quint16 is already limited to 0-65535
}

bool Receiver::createSocket(int &fd, int type) const
{
    fd = socket(AF_INET, type, 0);
    if (fd < 0) {
        qCritical() << "Socket creation failed for type" << type;
        return false;
    }
    return true;
}

void Receiver::closeSocket(int &fd)
{
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

bool Receiver::connectTcp(quint16 port)
{
    disconnect(); // Close existing connection
    
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
    
    connect(receiverThread, &ReceiverThread::jsonReceived, this, &Receiver::jsonReceived);
    receiverThread->start();

    qInfo() << "✅ TCP Receiver listening on port" << port;
    return true;
}

bool Receiver::connectUdp(quint16 port)
{
    disconnect(); // Close existing connection
    
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
    
    connect(receiverThread, &ReceiverThread::jsonReceived, this, &Receiver::jsonReceived);
    receiverThread->start();

    qInfo() << "✅ UDP Receiver listening on port" << port;
    return true;
}

void Receiver::disconnect()
{
    if (receiverThread) {
        receiverThread->stop();
        receiverThread->wait();
        delete receiverThread;
        receiverThread = nullptr;
    }
    closeSocket(listen_fd);
}
