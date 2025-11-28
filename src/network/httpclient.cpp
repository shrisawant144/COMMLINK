#include "commlink/network/httpclient.h"
#include <QNetworkRequest>
#include <QDateTime>

HttpClient::HttpClient(QObject *parent)
    : QObject(parent), m_format(DataFormatType::JSON), m_method(POST) {
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &HttpClient::onReplyFinished);
}

void HttpClient::sendRequest(const QString& url, Method method, const DataMessage& message) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray data = message.serialize();

    switch (method) {
        case GET:
            m_manager->get(request);
            break;
        case POST:
            m_manager->post(request, data);
            break;
        case PUT:
            m_manager->put(request, data);
            break;
        case DELETE:
            m_manager->deleteResource(request);
            break;
    }
}

void HttpClient::onReplyFinished(QNetworkReply* reply) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString source = reply->url().toString();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        DataMessage msg = DataMessage::deserialize(data, m_format);
        emit responseReceived(msg, source, timestamp);
    } else {
        emit errorOccurred(reply->errorString());
    }
    reply->deleteLater();
}
