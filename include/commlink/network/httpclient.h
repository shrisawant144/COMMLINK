#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "../core/dataformat.h"

class HttpClient : public QObject {
    Q_OBJECT
public:
    enum Method { GET, POST, PUT, DELETE };

    explicit HttpClient(QObject *parent = nullptr);
    
    void sendRequest(const QString& url, Method method, const DataMessage& message = DataMessage());
    void setFormat(DataFormatType format) { m_format = format; }
    void setMethod(Method method) { m_method = method; }

signals:
    void responseReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager *m_manager;
    DataFormatType m_format;
    Method m_method;
};

#endif
