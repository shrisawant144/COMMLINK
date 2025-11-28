#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include "../core/dataformat.h"

class HttpServer : public QObject {
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);
    
    bool startServer(quint16 port);
    void stopServer();
    bool isListening() const;
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
    struct HttpRequest {
        QString method;
        QString path;
        QMap<QString, QString> headers;
        QByteArray body;
    };
    
    HttpRequest parseRequest(const QByteArray& data);
    QByteArray buildResponse(int statusCode, const QString& body);
    
    QTcpServer *m_server;
    DataFormatType m_format;
    QMap<QTcpSocket*, QString> m_clients;
};

#endif
