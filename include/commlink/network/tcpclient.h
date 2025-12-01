#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include "../core/dataformat.h"

class TcpClient : public QObject {
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    
    void connectToHost(const QString& host, quint16 port);
    void disconnect();
    void sendMessage(const DataMessage& message);
    bool isConnected() const;
    void setFormat(DataFormatType format) { m_format = format; }

signals:
    void connected();
    void disconnected();
    void messageReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onConnectionTimeout();

private:
    QTcpSocket *m_socket;
    QTimer *m_connectionTimer;
    DataFormatType m_format;
    static const int CONNECTION_TIMEOUT_MS = 3000;
};

#endif
