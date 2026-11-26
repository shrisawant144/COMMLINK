#include "commlink/core/replayengine.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>

ReplayEngine::ReplayEngine(QObject* parent) : QObject(parent) {
    schedule.setSingleShot(true);
    timeout.setSingleShot(true);
    connect(&schedule, &QTimer::timeout, this, &ReplayEngine::sendNext);
    connect(&timeout, &QTimer::timeout, this,
            [this] { finish(false, "Response assertion timed out"); });
}
bool ReplayEngine::start(const QJsonObject& configuration) {
    if (active)
        return false;
    config = configuration;
    total = config["count"].toInt(1);
    if (total < 1 || total > 10000 || config["intervalMs"].toInt(1000) < 10 ||
        config["timeoutMs"].toInt(5000) < 10)
        return false;
    const int assertion = config["assertion"].toInt();
    if (assertion < 0 || assertion > 3)
        return false;
    if (config["intervalMs"].toInt(1000) > 60000 || config["timeoutMs"].toInt(5000) > 300000)
        return false;
    if (assertion == 2) {
        QString expected = config["expected"].toString();
        expected.remove(QRegularExpression("\\s"));
        if (expected.size() % 2 || !QRegularExpression("^[0-9a-fA-F]*$").match(expected).hasMatch())
            return false;
    }
    completed = 0;
    active = true;
    waiting = false;
    schedule.start(0);
    return true;
}
void ReplayEngine::sendNext() {
    if (!active)
        return;
    waiting = config["assertion"].toInt() != 0;
    if (waiting)
        timeout.start(config["timeoutMs"].toInt(5000));
    emit sendRequested();
    if (active && !waiting && !schedule.isActive())
        completeStep();
}
void ReplayEngine::completeStep() {
    timeout.stop();
    waiting = false;
    ++completed;
    emit progress(completed, total);
    if (completed >= total)
        finish(true, QString("Replay passed: %1/%2 sends").arg(completed).arg(total));
    else
        schedule.start(config["intervalMs"].toInt(1000));
}
void ReplayEngine::receive(const QByteArray& payload) {
    if (!active || !waiting)
        return;
    const int assertion = config["assertion"].toInt();
    const auto expected = config["expected"].toString();
    bool passed = false;
    if (assertion == 1)
        passed = payload.contains(expected.toUtf8());
    if (assertion == 2)
        passed = payload == QByteArray::fromHex(expected.toLatin1());
    if (assertion == 3) {
        QJsonParseError error;
        const auto doc = QJsonDocument::fromJson(payload, &error);
        QJsonValue value = doc.isArray() ? QJsonValue(doc.array()) : QJsonValue(doc.object());
        for (const auto& part : config["path"].toString().split('.', Qt::SkipEmptyParts)) {
            if (value.isArray()) {
                bool ok = false;
                const int index = part.toInt(&ok);
                value = ok && index >= 0 && index < value.toArray().size()
                            ? value.toArray().at(index)
                            : QJsonValue(QJsonValue::Undefined);
            } else
                value = value.toObject().value(part);
        }
        const auto expectedDoc = QJsonDocument::fromJson(("[" + expected + "]").toUtf8());
        const QJsonValue expectedValue = expectedDoc.isArray() && expectedDoc.array().size() == 1
                                             ? expectedDoc.array().first()
                                             : QJsonValue(expected);
        passed = error.error == QJsonParseError::NoError && !value.isUndefined() &&
                 value == expectedValue;
    }
    if (passed)
        completeStep();
    else
        finish(false, "Response assertion failed on send " + QString::number(completed + 1));
}
void ReplayEngine::finish(bool success, const QString& description) {
    if (!active)
        return;
    active = false;
    waiting = false;
    schedule.stop();
    timeout.stop();
    emit finished(success, description);
}
void ReplayEngine::stop(const QString& reason) {
    finish(false, reason);
}
