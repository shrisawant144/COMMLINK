#pragma once
#include <QJsonObject>
#include <QObject>
#include <QTimer>

class ReplayEngine : public QObject {
    Q_OBJECT
public:
    explicit ReplayEngine(QObject* parent = nullptr);
    bool start(const QJsonObject& configuration);
    void receive(const QByteArray& payload);
    void stop(const QString& reason = "Stopped by user");
    bool isActive() const {
        return active;
    }
signals:
    void sendRequested();
    void progress(int completed, int total);
    void finished(bool success, const QString& description);

private:
    void sendNext();
    void completeStep();
    void finish(bool success, const QString& description);
    QTimer schedule, timeout;
    QJsonObject config;
    int completed = 0, total = 0;
    bool active = false, waiting = false;
};
