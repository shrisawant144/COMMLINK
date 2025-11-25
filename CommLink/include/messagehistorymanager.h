#pragma once

#include <QtCore/QObject>
#include <QtSql/QSqlDatabase>
#include <QtCore/QJsonDocument>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QUuid>
#include <QtCore/QMutex>

class MessageHistoryManager : public QObject
{
    Q_OBJECT

public:
    explicit MessageHistoryManager(QObject *parent = nullptr);
    ~MessageHistoryManager();

    bool initializeDatabase();
    bool saveMessage(const QString &direction, const QString &protocol,
                     const QString &host, int port, const QJsonDocument &content,
                     const QString &senderInfo = QString());
    QList<QVariantMap> getMessages(const QString &filter = QString(),
                                   const QDateTime &startDate = QDateTime(),
                                   const QDateTime &endDate = QDateTime(),
                                   int limit = 100, int offset = 0);
    bool deleteOldMessages(int daysToKeep);
    bool exportMessages(const QString &filePath, const QString &format = "json");
    bool clearAllMessages();
    int getMessageCount(const QString &filter = QString());
    QDateTime getLastMessageTime();

    QString getCurrentSessionId() const { return currentSessionId; }
    void startNewSession();

private:
    QSqlDatabase db;
    QString currentSessionId;
    QDateTime lastUpdateTime;
    mutable QMutex dbMutex; // Thread safety

    bool createTables();
    bool executeQuery(const QString &query);
    QString generateSessionId();
    bool optimizeDatabase();
};
