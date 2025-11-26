#pragma once
#include <QObject>
#include <QDateTime>
#include <atomic>

class ConnectionStats : public QObject
{
    Q_OBJECT
public:
    ConnectionStats(QObject *parent = nullptr);
    
    void recordSent();
    void recordReceived();
    void connectionStarted();
    void connectionStopped();
    
    quint64 messagesSent() const { return sent; }
    quint64 messagesReceived() const { return received; }
    qint64 uptimeSeconds() const;
    
signals:
    void statsUpdated();
    
private:
    std::atomic<quint64> sent{0};
    std::atomic<quint64> received{0};
    QDateTime connectionStart;
};