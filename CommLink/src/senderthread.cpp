#include "../include/senderthread.h"
#include <QDebug>

SenderThread::SenderThread(QObject *parent) : QThread(parent) {}

SenderThread::~SenderThread()
{
    stop();
    wait();
}

void SenderThread::queueMessage(const DataMessage &msg)
{
    QMutexLocker locker(&queueMutex);
    messageQueue.enqueue(msg);
    queueCondition.wakeOne();
}

void SenderThread::stop()
{
    running = false;
    queueCondition.wakeAll();
}

void SenderThread::run()
{
    while (running) {
        QMutexLocker locker(&queueMutex);
        
        if (messageQueue.isEmpty()) {
            queueCondition.wait(&queueMutex, 100);
            continue;
        }
        
        DataMessage msg = messageQueue.dequeue();
        locker.unlock();
        
        emit messageSent(msg);
        msleep(10);
    }
}