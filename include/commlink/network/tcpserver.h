#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "../core/dataformat.h"
#include "../core/streamframer.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QSslSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

class TcpServer : public QObject {
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();
    bool startServer(quint16 port, const QHostAddress& bindAddress = QHostAddress::Any);
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

    void setFraming(StreamFramer::Mode mode, const QByteArray& delimiter = "\n");
    void setKeepAlive(bool enabled);
    quint16 localPort() const;

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
    bool readBatch(QTcpSocket* client);
    void scheduleRead(QTcpSocket* client);
    QSet<QTcpSocket*> scheduledReads;
    QMap<QTcpSocket*, QString> peerEndpoints;
    StreamFramer m_framing;
    QMap<QTcpSocket*, StreamFramer> m_framers;
    bool m_keepAlive = false;
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
