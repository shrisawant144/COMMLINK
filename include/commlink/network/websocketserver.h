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
    
    bool listen(quint16 port);
    void close();
    bool isListening() const;
    void setFormat(DataFormatType format) { m_format = format; }

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
};

#endif
