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
    void setSSLEnabled(bool enabled) { m_sslEnabled = enabled; }
    bool isSSLEnabled() const { return m_sslEnabled; }

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
    
    static QByteArray buildResponse(int statusCode, const QByteArray& body, DataFormatType format);
    static QByteArray buildCORSPreflightResponse();
    bool tryParseCompleteRequest(QTcpSocket* socket);
    DataFormatType detectContentType(const QString& contentType);
    DataFormatType detectAcceptType(const QString& accept);
    QByteArray buildResponseBody(const HttpRequest& request, DataFormatType format);
    
    QTcpServer *m_server;
    DataFormatType m_format;
    bool m_sslEnabled;
    QMap<QTcpSocket*, QString> m_clients;
    QMap<QTcpSocket*, QByteArray> m_requestBuffers;
         static constexpr int MAX_CLIENTS = 100;
         static constexpr int MAX_BUFFER_SIZE = 8192;
};

#endif
