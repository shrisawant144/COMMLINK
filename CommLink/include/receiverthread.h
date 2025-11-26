#pragma once
#include <QThread>
#include <QJsonDocument>
#include <QString>
#include <sys/socket.h>
#include <netinet/in.h>
#include <atomic>
#include "dataformat.h"

class ReceiverThread : public QThread
{
    Q_OBJECT
public:
    ReceiverThread(int socket_fd, bool is_tcp, QObject *parent = nullptr);
    ~ReceiverThread();

    void stop();

signals:
    void dataReceived(const DataMessage &msg, const QString &protocol, const QString &senderInfo);

protected:
    void run() override;

private:
    QString formatSenderInfo(const sockaddr_in &addr) const;
    bool processReceivedData(const char *buffer, ssize_t size, const QString &senderInfo);
    
    int socket_fd;
    bool is_tcp;
    std::atomic<bool> running{true};
    static constexpr size_t BUFFER_SIZE = 4096;
};
