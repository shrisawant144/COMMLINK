#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMap>
#include <QTimer>
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
    void setConnected(bool connected);
    void disconnect();
    
    // Long-polling support
    void startPolling(const QString& url, int intervalMs = 2000);
    void stopPolling();
    bool isPolling() const { return m_isPolling; }
    void setPollTimeout(int msecs) { m_pollTimeout = msecs; }
    int getPollTimeout() const { return m_pollTimeout; }
    
    static QString methodToString(Method method);

signals:
    void connected();
    void disconnected();
    void responseReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);
    void requestSent(const QString& method, const QString& url);
    void pollingStopped(const QString& reason);

private slots:
    void onReplyFinished(QNetworkReply* reply);
    void onPollTimeout();

private:
    QNetworkRequest buildRequest(const QString& url);
    QString getContentType() const;
    void sendPollRequest();
    
    QNetworkAccessManager *m_manager;
    DataFormatType m_format;
    Method m_method;
    QMap<QString, QString> m_headers;
    int m_timeout;
    bool m_connected;
    
    // Long-polling members
    bool m_isPolling;
    QString m_pollUrl;
    int m_pollInterval;
    int m_pollTimeout;
    int m_consecutiveErrors;
    QTimer *m_pollTimer;
    
    static constexpr int DEFAULT_TIMEOUT_MS = 30000;
    static constexpr int MAX_POLL_ERRORS = 3;
};

#endif
