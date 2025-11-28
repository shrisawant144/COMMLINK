#include "commlink/receiverthread.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

ReceiverThread::ReceiverThread(int socket_fd, bool is_tcp, QObject *parent)
    : QThread(parent), socket_fd(socket_fd), is_tcp(is_tcp)
{
}

ReceiverThread::~ReceiverThread()
{
    stop();
    wait();
}

void ReceiverThread::stop()
{
    running = false;
    // Don't close socket here - let the parent handle it
    // This prevents race conditions
}

QString ReceiverThread::formatSenderInfo(const sockaddr_in &addr) const
{
    char ip[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip)) == nullptr) {
        return "unknown";
    }
    return QString("%1:%2").arg(ip).arg(ntohs(addr.sin_port));
}

bool ReceiverThread::processReceivedData(const char *buffer, ssize_t size, const QString &senderInfo)
{
    if (size <= 0) return false;

    QByteArray data = QByteArray::fromRawData(buffer, static_cast<int>(size)).trimmed();
    if (data.isEmpty()) return false;

    // Try to detect format and deserialize accordingly
    DataMessage msg;
    QString dataStr = QString::fromUtf8(data);
    
    // Try JSON first
    if (DataMessage::validateInput(dataStr, DataFormatType::JSON)) {
        msg = DataMessage::deserialize(data, DataFormatType::JSON);
    }
    // Try XML
    else if (dataStr.contains('<') && dataStr.contains('>')) {
        msg = DataMessage::deserialize(data, DataFormatType::XML);
    }
    // Try CSV (contains commas)
    else if (dataStr.contains(',')) {
        msg = DataMessage::deserialize(data, DataFormatType::CSV);
    }
    // Default to TEXT
    else {
        msg = DataMessage::deserialize(data, DataFormatType::TEXT);
    }
    
    emit dataReceived(msg, is_tcp ? "TCP" : "UDP", senderInfo);
    return true;
}

void ReceiverThread::run()
{
    char buffer[BUFFER_SIZE];
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    // Set socket to non-blocking mode
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

    while (running) {
        ssize_t received = -1;

        if (is_tcp) {
            int client_fd = accept(socket_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
            if (client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    msleep(10); // Brief sleep to prevent busy waiting
                    continue;
                }
                if (running) qWarning() << "TCP accept failed:" << strerror(errno);
                continue;
            }

            // Keep the connection open and receive multiple messages
            while (running) {
                received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
                if (received <= 0) {
                    if (received < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                        msleep(10);
                        continue;
                    }
                    break; // Connection closed or error
                }
                buffer[received] = '\0';
                QString senderInfo = formatSenderInfo(client_addr);
                processReceivedData(buffer, received, senderInfo);
            }
            close(client_fd);
        } else {
            received = recvfrom(socket_fd, buffer, BUFFER_SIZE - 1, 0,
                              reinterpret_cast<sockaddr*>(&client_addr), &client_len);
            if (received < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                msleep(10); // Brief sleep to prevent busy waiting
                continue;
            }
        }

        if (received > 0) {
            buffer[received] = '\0';
            QString senderInfo = formatSenderInfo(client_addr);
            processReceivedData(buffer, received, senderInfo);
        } else if (received < 0 && running && errno != EAGAIN && errno != EWOULDBLOCK) {
            qWarning() << (is_tcp ? "TCP" : "UDP") << "receive failed:" << strerror(errno);
        }
    }
}
