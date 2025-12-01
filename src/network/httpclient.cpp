#include "commlink/network/httpclient.h"
#include <QNetworkRequest>
#include <QDateTime>
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

    if (reply != nullptr && m_timeout > 0) {
        QTimer::singleShot(m_timeout, reply, &QNetworkReply::abort);
    }
}

void HttpClient::addHeader(const QString& key, const QString& value) {
    m_headers[key] = value;
}

void HttpClient::clearHeaders() {
    m_headers.clear();
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

void HttpClient::onReplyFinished(QNetworkReply* reply) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString source = reply->url().toString();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() == QNetworkReply::NoError) {
        // Reset error counter on successful response
        if (m_isPolling) {
            m_consecutiveErrors = 0;
        }
        
        QByteArray data = reply->readAll();
        
        // Detect format from Content-Type header
        DataFormatType responseFormat = m_format;
        QVariant contentTypeVar = reply->header(QNetworkRequest::ContentTypeHeader);
        if (contentTypeVar.isValid()) {
            QString contentType = contentTypeVar.toString().toLower();
            // Remove charset and other parameters
            contentType = contentType.split(';').first().trimmed();
            
            if (contentType == "application/json") {
                responseFormat = DataFormatType::JSON;
            } else if (contentType == "application/xml" || contentType == "text/xml") {
                responseFormat = DataFormatType::XML;
            } else if (contentType == "text/csv") {
                responseFormat = DataFormatType::CSV;
            } else if (contentType == "text/plain") {
                responseFormat = DataFormatType::TEXT;
            } else if (contentType == "application/octet-stream") {
                responseFormat = DataFormatType::BINARY;
            }
        }
        
        DataMessage msg = DataMessage::deserialize(data, responseFormat);
        
        QString statusInfo = QString(" [HTTP %1]").arg(statusCode);
        emit responseReceived(msg, source + statusInfo, timestamp);
    } else {
        // Handle errors
        if (m_isPolling) {
            m_consecutiveErrors++;
            if (m_consecutiveErrors >= MAX_POLL_ERRORS) {
                stopPolling();
                QString reason = QString("Server not responding after %1 attempts").arg(MAX_POLL_ERRORS);
                emit pollingStopped(reason);
                emit errorOccurred(QString("Polling stopped: %1").arg(reason));
                return;
            }
        }
        
        QString error = QString("HTTP Error %1: %2").arg(statusCode).arg(reply->errorString());
        emit errorOccurred(error);
    }
    
    // Don't auto-disconnect after each request - HTTP can send multiple requests
    // Only disconnect when explicitly called via disconnect()
    reply->deleteLater();
}

void HttpClient::disconnect() {
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
    if (!m_pollUrl.isEmpty()) {
        // Always use GET for polling requests
        QNetworkRequest request = buildRequest(m_pollUrl);
        
        // Set custom timeout for polling
        QNetworkReply* reply = m_manager->get(request);
        
        if (reply && m_pollTimeout > 0) {
            QTimer::singleShot(m_pollTimeout, reply, &QNetworkReply::abort);
        }
    }
}
