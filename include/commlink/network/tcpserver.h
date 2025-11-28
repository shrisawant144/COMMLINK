#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include "../core/dataformat.h"

class TcpServer : public QObject {
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();
    
    bool listen(quint16 port);
    void close();
    bool isListening() const;
    void sendToAll(const DataMessage& message);
    void setFormat(DataFormatType format) { m_format = format; }

signals:
    void clientConnected(const QString& clientInfo);
    void clientDisconnected(const QString& clientInfo);
    void messageReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients;
    DataFormatType m_format;
};

#endif
