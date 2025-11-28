#pragma once
#include <QtCore/QObject>
#include <QtCore/QJsonDocument>
#include <sys/socket.h>
#include <netinet/in.h>
#include "dataformat.h"

class ReceiverThread;

class Receiver : public QObject
{
    Q_OBJECT
public:
    Receiver() = default;
    ~Receiver();

    bool connectTcp(quint16 port);
    bool connectUdp(quint16 port);
    void disconnect();

    bool isConnected() const { return listen_fd >= 0; }

signals:
    void dataReceived(const DataMessage &msg, const QString &protocol, const QString &senderInfo);

private:
    bool validatePort(quint16 port) const;
    bool createSocket(int &fd, int type) const;
    void closeSocket(int &fd);
    
    int listen_fd = -1;
    ReceiverThread *receiverThread = nullptr;
};
