#pragma once
#include <QtCore/QObject>
#include <QtCore/QJsonDocument>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>

class Sender : public QObject
{
    Q_OBJECT
public:
    enum class ConnectionType { TCP, UDP };
    
    Sender() = default;
    ~Sender();

    std::function<void(const QJsonDocument&)> sendJson;

    bool connectTcp(const QString &host, quint16 port);
    bool connectUdp(const QString &host, quint16 port);
    void disconnect();
    
    bool isConnected() const { return tcp_fd >= 0 || udp_fd >= 0; }
    ConnectionType getConnectionType() const;

private:
    bool validatePort(quint16 port) const;
    bool createSocket(int &fd, int type) const;
    bool setupAddress(const QString &host, quint16 port, sockaddr_in &addr) const;
    void closeSocket(int &fd);
    
    int tcp_fd = -1;
    int udp_fd = -1;
    sockaddr_in udp_addr{};
};
