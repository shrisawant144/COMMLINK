#include "commlink/network/connectionstats.h"

ConnectionStats::ConnectionStats(QObject *parent) : QObject(parent) {}

void ConnectionStats::recordSent()
{
    sent++;
    emit statsUpdated();
}

void ConnectionStats::recordReceived()
{
    received++;
    emit statsUpdated();
}

void ConnectionStats::connectionStarted()
{
    connectionStart = QDateTime::currentDateTime();
    sent = 0;
    received = 0;
    emit statsUpdated();
}

void ConnectionStats::connectionStopped()
{
    connectionStart = QDateTime();
    emit statsUpdated();
}

qint64 ConnectionStats::uptimeSeconds() const
{
    if (!connectionStart.isValid()) return 0;
    return connectionStart.secsTo(QDateTime::currentDateTime());
}