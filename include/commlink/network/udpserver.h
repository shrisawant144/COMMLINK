#ifndef UDPSERVER_H
#define UDPSERVER_H

#include "../core/dataformat.h"

#include <QMap>
#include <QObject>
#include <QUdpSocket>

class UdpServer : public QObject {
    Q_OBJECT
public:
    explicit UdpServer(QObject *parent = nullptr);
    bool startServer(quint16 port, const QHostAddress& bindAddress = QHostAddress::Any);
    void stopServer();
    bool isListening() const { return m_listening; }
    void setFormat(DataFormatType format) { m_format = format; }
    quint16 localPort() const {
        return m_socket->localPort();
    }
    void sendToAll(const DataMessage& message);
    void sendTo(const QHostAddress& address, quint16 port, const DataMessage& message);

signals:
    void peerDiscovered(const QString& endpoint);
    void messageReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();

private:
    QMap<QString, QPair<QHostAddress, quint16>> peers;
    QUdpSocket *m_socket;
    bool m_listening;
    DataFormatType m_format;
    static constexpr int MAX_BUFFER_SIZE = 65507;
};

#endif
