#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMap>
#include "../core/dataformat.h"

class HttpClient : public QObject {
    Q_OBJECT
public:
    enum Method { GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS };

    explicit HttpClient(QObject *parent = nullptr);
    
    void sendRequest(const QString& url, Method method, const DataMessage& message = DataMessage());
    void setFormat(DataFormatType format) { m_format = format; }
    void setMethod(Method method) { m_method = method; }
    void addHeader(const QString& key, const QString& value);
    void clearHeaders();
    void setTimeout(int msecs) { m_timeout = msecs; }
    bool isConnected() const { return m_connected; }
    void disconnect() { m_connected = false; }
    
    static QString methodToString(Method method);

signals:
    void connected();
    void disconnected();
    void responseReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);
    void requestSent(const QString& method, const QString& url);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkRequest buildRequest(const QString& url);
    QString getContentType() const;
    
    QNetworkAccessManager *m_manager;
    DataFormatType m_format;
    Method m_method;
    QMap<QString, QString> m_headers;
    int m_timeout;
    bool m_connected;
};

#endif
