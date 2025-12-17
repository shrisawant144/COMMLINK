#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>
#include "../core/dataformat.h"

class WebSocketServer : public QObject {
    Q_OBJECT
public:
    explicit WebSocketServer(QObject *parent = nullptr);
    ~WebSocketServer();
    bool startServer(quint16 port);
    void stopServer();
    bool isListening() const;
    void setFormat(DataFormatType format) { m_format = format; }
    void sendToClient(QWebSocket* client, const DataMessage& message, bool binary = false);
    void sendToAll(const DataMessage& message, bool binary = false);
    QWebSocket* findClientByAddress(const QString& addressPort);
    void setSSLEnabled(bool enabled) { m_sslEnabled = enabled; }
    bool isSSLEnabled() const { return m_sslEnabled; }

signals:
    void clientConnected(const QString& clientInfo);
    void clientDisconnected(const QString& clientInfo);
    void messageReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& message);
    void onClientDisconnected();

private:
    QWebSocketServer *m_server;
    QList<QWebSocket*> m_clients;
    DataFormatType m_format;
    bool m_sslEnabled;
         static constexpr int MAX_CLIENTS = 100;
         static constexpr int MAX_BUFFER_SIZE = 8192;
};

#endif
