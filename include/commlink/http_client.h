#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QByteArray>

namespace commlink {

class HttpClient : public QObject {
    Q_OBJECT

public:
    enum Method { GET, POST, PUT, DELETE };

    explicit HttpClient(QObject* parent = nullptr);
    ~HttpClient() override = default;

    void sendRequest(const QString& url, Method method, const QByteArray& data = QByteArray());

signals:
    void responseReceived(int statusCode, const QByteArray& data);
    void errorOccurred(const QString& error);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_manager;
};

} // namespace commlink
