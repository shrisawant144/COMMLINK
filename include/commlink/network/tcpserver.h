#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSslSocket>
#include <QList>
#include <QTimer>
#include <QMap>
#include "../core/dataformat.h"

class TcpServer : public QObject {
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();
    bool startServer(quint16 port);
    void stopServer();
    bool isListening() const;
    void sendToAll(const DataMessage& message);
    void sendToClient(QTcpSocket* client, const DataMessage& message);
    QTcpSocket* findClientByAddress(const QString& addressPort);
    void setFormat(DataFormatType format) { m_format = format; }
    void setSSLEnabled(bool enabled) { m_sslEnabled = enabled; }
    bool isSSLEnabled() const { return m_sslEnabled; }
    void setIdleTimeout(int seconds) { m_idleTimeout = seconds; }
    int getIdleTimeout() const { return m_idleTimeout; }

signals:
    void clientConnected(const QString& clientInfo);
    void clientDisconnected(const QString& clientInfo);
    void messageReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();
    void checkIdleConnections();

private:
    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients;
    QMap<QTcpSocket*, qint64> m_lastActivity;
    QTimer *m_idleTimer;
    DataFormatType m_format;
    bool m_sslEnabled;
    int m_idleTimeout;
    static constexpr int MAX_CLIENTS = 100;
    static constexpr int MAX_BUFFER_SIZE = 8192;
};

#endif
