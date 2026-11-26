#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include "../core/dataformat.h"

#include <QObject>
#include <QTimer>
#include <QWebSocket>

class WebSocketClient : public QObject {
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);
    
    void connectToServer(const QString& url);
    void disconnect();
    void sendMessage(const DataMessage& message);
    bool isConnected() const;
    void setPingInterval(int milliseconds);
    void ping(const QByteArray& payload = QByteArray());
    void setFormat(DataFormatType format) { m_format = format; }

signals:
    void diagnostic(const QString& description);
    void connected();
    void disconnected();
    void messageReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& message);
    void onError(QAbstractSocket::SocketError error);

private:
    QTimer pingTimer;
    QWebSocket m_socket;
    DataFormatType m_format;
    bool m_connected;
};

#endif
