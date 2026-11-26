#include "commlink/network/httpclient.h"

#include <QDateTime>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>

HttpClient::HttpClient(QObject *parent)
    : QObject(parent), m_format(DataFormatType::JSON), m_method(POST), 
      m_timeout(DEFAULT_TIMEOUT_MS), m_connected(false),
      m_isPolling(false), m_pollInterval(2000), m_pollTimeout(10000), m_consecutiveErrors(0) {
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &HttpClient::onReplyFinished);
    
    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &HttpClient::onPollTimeout);
}

void HttpClient::sendRequest(const QString& url, Method method, const DataMessage& message) {
    if (activeReplies.size() >= 16 || !headersValid) {
        emit errorOccurred("HTTP request limit reached or headers are invalid");
        return;
    }
    QNetworkRequest request = buildRequest(url);
    QByteArray data = message.serialize();
    
    m_connected = true;
    emit connected();
    emit requestSent(methodToString(method), url);

    QNetworkReply* reply = nullptr;
    switch (method) {
        case GET:
            reply = m_manager->get(request);
            break;
        case POST:
            reply = m_manager->post(request, data);
            break;
        case PUT:
            reply = m_manager->put(request, data);
            break;
        case DELETE:
            reply = m_manager->deleteResource(request);
            break;
        case PATCH:
            reply = m_manager->sendCustomRequest(request, "PATCH", data);
            break;
        case HEAD:
            reply = m_manager->head(request);
            break;
        case OPTIONS:
            reply = m_manager->sendCustomRequest(request, "OPTIONS");
            break;
    }

    if (reply)
        watchReply(reply);
}

void HttpClient::addHeader(const QString& key, const QString& value) {
    if (!QRegularExpression("^[!#$%&'*+.^_`|~0-9A-Za-z-]+$").match(key).hasMatch() ||
        value.contains('\r') || value.contains('\n')) {
        headersValid = false;
        emit errorOccurred("Invalid HTTP header");
        return;
    }
    m_headers[key] = value;
}

void HttpClient::clearHeaders() {
    m_headers.clear();
    headersValid = true;
}

QNetworkRequest HttpClient::buildRequest(const QString& url) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, getContentType());
    request.setHeader(QNetworkRequest::UserAgentHeader, "CommLink/1.0");
    
    // Add Accept header based on expected response format
    if (!m_headers.contains("Accept")) {
        request.setRawHeader("Accept", getContentType().toUtf8());
    }
    
    for (auto it = m_headers.constBegin(); it != m_headers.constEnd(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    
    return request;
}

QString HttpClient::getContentType() const {
    switch (m_format) {
        case DataFormatType::JSON: return "application/json";
        case DataFormatType::XML: return "application/xml";
        case DataFormatType::CSV: return "text/csv";
        case DataFormatType::TEXT: return "text/plain";
        case DataFormatType::BINARY: return "application/octet-stream";
        case DataFormatType::HEX: return "text/plain";
        default: return "application/json";
    }
}

QString HttpClient::methodToString(Method method) {
    switch (method) {
        case GET: return "GET";
        case POST: return "POST";
        case PUT: return "PUT";
        case DELETE: return "DELETE";
        case PATCH: return "PATCH";
        case HEAD: return "HEAD";
        case OPTIONS: return "OPTIONS";
        default: return "UNKNOWN";
    }
}

void HttpClient::watchReply(QNetworkReply* reply, bool polling) {
    activeReplies.insert(reply);
    reply->setProperty("polling", polling);
    reply->setProperty("started", QDateTime::currentMSecsSinceEpoch());
    QTimer::singleShot(polling ? m_pollTimeout : m_timeout, reply, [reply] {
        if (!reply->isFinished()) {
            reply->setProperty("timedOut", true);
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::readyRead, this, [this, reply] {
        auto& body = replyBodies[reply];
        const auto bytes = reply->readAll();
        if (body.size() + bytes.size() > 8 * 1024 * 1024) {
            reply->setProperty("oversized", true);
            reply->abort();
            return;
        }
        body += bytes;
    });
}
void HttpClient::onReplyFinished(QNetworkReply* reply) {
    activeReplies.remove(reply);
    const QByteArray body = replyBodies.take(reply) + reply->readAll();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QVariantMap headers;
    for (const auto& pair : reply->rawHeaderPairs())
        headers[QString::fromLatin1(pair.first)] = QString::fromLatin1(pair.second);
    QString error;
    if (reply->property("oversized").toBool())
        error = "HTTP response exceeds 8 MiB";
    else if (reply->property("timedOut").toBool())
        error = "HTTP request timed out";
    else if (reply->error() != QNetworkReply::NoError)
        error = QString("HTTP %1: %2").arg(status).arg(reply->errorString());
    const qint64 elapsed = qMax(qint64(0), QDateTime::currentMSecsSinceEpoch() -
                                               reply->property("started").toLongLong());
    emit responseInspected(body, reply->url().toString(), status, headers, elapsed, error);
    if (!reply->property("oversized").toBool() &&
        (status > 0 || reply->error() == QNetworkReply::NoError)) {
        DataFormatType format = m_format;
        const auto contentType = reply->header(QNetworkRequest::ContentTypeHeader)
                                     .toString()
                                     .toLower()
                                     .split(';')
                                     .first()
                                     .trimmed();
        if (contentType == "application/json")
            format = DataFormatType::JSON;
        else if (contentType == "application/xml" || contentType == "text/xml")
            format = DataFormatType::XML;
        else if (contentType == "application/octet-stream")
            format = DataFormatType::BINARY;
        else if (contentType == "text/plain")
            format = DataFormatType::TEXT;
        else if (contentType == "text/csv")
            format = DataFormatType::CSV;
        emit responseReceived(DataMessage::deserialize(body, format),
                              reply->url().toString() + QString(" [HTTP %1]").arg(status),
                              QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    }
    const bool polling = reply->property("polling").toBool();
    reply->deleteLater();
    if (!error.isEmpty()) {
        emit errorOccurred(error);
        if (polling && m_isPolling && ++m_consecutiveErrors >= MAX_POLL_ERRORS) {
            stopPolling();
            emit pollingStopped(error);
        }
    } else if (polling)
        m_consecutiveErrors = 0;
}

void HttpClient::disconnect() {
    stopPolling();
    const auto replies = activeReplies;
    for (auto* reply : replies)
        reply->abort();
    if (m_connected) {
        m_connected = false;
        emit disconnected();
    }
}

void HttpClient::setConnected(bool connected) {
    if (m_connected != connected) {
        m_connected = connected;
        if (connected) {
            emit this->connected();
        } else {
            emit disconnected();
        }
    }
}

void HttpClient::startPolling(const QString& url, int intervalMs) {
    m_pollUrl = url;
    m_pollInterval = intervalMs;
    m_isPolling = true;
    m_consecutiveErrors = 0;
    
    // Send first poll request immediately
    sendPollRequest();
    
    // Start timer for subsequent polls
    m_pollTimer->start(m_pollInterval);
}

void HttpClient::stopPolling() {
    m_isPolling = false;
    m_consecutiveErrors = 0;
    m_pollTimer->stop();
}

void HttpClient::onPollTimeout() {
    if (m_isPolling) {
        sendPollRequest();
    }
}

void HttpClient::sendPollRequest() {
    for (auto* reply : activeReplies)
        if (reply->property("polling").toBool())
            return;
    if (!m_pollUrl.isEmpty()) {
        // Always use GET for polling requests
        QNetworkRequest request = buildRequest(m_pollUrl);
        
        // Set custom timeout for polling
        QNetworkReply* reply = m_manager->get(request);

        if (reply)
            watchReply(reply, true);
    }
}
