#include "sender.h"
#include <QDebug>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

Sender::~Sender() {
    if (tcp_fd >= 0) close(tcp_fd);
    if (udp_fd >= 0) close(udp_fd);
}

bool Sender::connectTcp(const QString &host, quint16 port)
{
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        qCritical() << "TCP socket creation failed";
        return false;
    }
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.toUtf8().constData(), &addr.sin_addr);
    
    if (::connect(tcp_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(tcp_fd);
        qCritical() << "TCP connect failed";
        return false;
    }
    
    qInfo() << "✅ RAW TCP:" << host << ":" << port;

    sendJson = [this](const QJsonDocument &doc) {
        QByteArray payload = doc.toJson(QJsonDocument::Compact) + "\n";
        ssize_t written = send(tcp_fd, payload.constData(), payload.size(), 0);
        if (written == payload.size()) qInfo() << "→ TCP:" << payload.constData();
        else qWarning() << "TCP partial send!";
    };
    return true;
}

bool Sender::connectUdp(const QString &host, quint16 port)
{
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        qCritical() << "UDP socket creation failed";
        return false;
    }
    
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(port);
    inet_pton(AF_INET, host.toUtf8().constData(), &udp_addr.sin_addr);
    
    qInfo() << "✅ RAW UDP:" << host << ":" << port;

    sendJson = [this](const QJsonDocument &doc) {
        QByteArray payload = doc.toJson(QJsonDocument::Compact) + "\n";
        ssize_t written = sendto(udp_fd, payload.constData(), payload.size(), 0, 
                               (sockaddr*)&udp_addr, sizeof(udp_addr));
        if (written == payload.size()) qInfo() << "→ UDP:" << payload.constData();
        else qWarning() << "UDP send failed!";
    };
    return true;
}