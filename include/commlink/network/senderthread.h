#pragma once
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>
#include "../core/dataformat.h"

class SenderThread : public QThread
{
    Q_OBJECT
public:
    SenderThread(QObject *parent = nullptr);
    ~SenderThread();

    void queueMessage(const DataMessage &msg);
    void stop();

signals:
    void messageSent(const DataMessage &msg);
    void sendError(const QString &error);

protected:
    void run() override;

private:
    QQueue<DataMessage> messageQueue;
    QMutex queueMutex;
    QWaitCondition queueCondition;
    std::atomic<bool> running{true};
};