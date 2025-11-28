#include "commlink/http_client.h"
#include <QNetworkRequest>

namespace commlink {

HttpClient::HttpClient(QObject* parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)) {
    connect(m_manager, &QNetworkAccessManager::finished, this, &HttpClient::onReplyFinished);
}

void HttpClient::sendRequest(const QString& url, Method method, const QByteArray& data) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

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
    if (reply->error() == QNetworkReply::NoError) {
        emit responseReceived(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                            reply->readAll());
    } else {
        emit errorOccurred(reply->errorString());
    }
    reply->deleteLater();
}

} // namespace commlink
