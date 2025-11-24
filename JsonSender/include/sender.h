#pragma once
#include <QObject>
#include <QJsonDocument>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>

class ReceiverThread;

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

    bool startTcpReceiver(quint16 port);
    bool startUdpReceiver(quint16 port);
    void stopReceiver();
    
    bool isConnected() const { return tcp_fd >= 0 || udp_fd >= 0; }
    ConnectionType getConnectionType() const;

signals:
    void jsonReceived(const QJsonDocument &doc, const QString &protocol, const QString &senderInfo);

private:
    bool validatePort(quint16 port) const;
    bool createSocket(int &fd, int type) const;
    bool setupAddress(const QString &host, quint16 port, sockaddr_in &addr) const;
    void closeSocket(int &fd);
    
    int tcp_fd = -1;
    int udp_fd = -1;
    int listen_fd = -1;
    sockaddr_in udp_addr{};
    ReceiverThread *receiverThread = nullptr;
};
