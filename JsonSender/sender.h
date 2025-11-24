#pragma once
#include <QObject>
#include <QJsonDocument>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>

class Sender : public QObject
{
    Q_OBJECT
public:
    Sender() = default;
    ~Sender();

    std::function<void(const QJsonDocument&)> sendJson;
    
    bool connectTcp(const QString &host, quint16 port);
    bool connectUdp(const QString &host, quint16 port);

private:
    int tcp_fd = -1;
    int udp_fd = -1;
    sockaddr_in udp_addr{};
};