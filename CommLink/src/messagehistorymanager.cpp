#include "../include/messagehistorymanager.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtCore/QVariant>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QMutexLocker>

MessageHistoryManager::MessageHistoryManager(QObject *parent)
    : QObject(parent)
{
    startNewSession();
}

MessageHistoryManager::~MessageHistoryManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool MessageHistoryManager::initializeDatabase()
{
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    dbPath += "/message_history.db";

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qCritical() << "Failed to open database:" << db.lastError().text();
        return false;
    }

    return createTables();
}

bool MessageHistoryManager::createTables()
{
    QString createMessagesTable = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            direction TEXT CHECK(direction IN ('sent', 'received')),
            protocol TEXT CHECK(protocol IN ('TCP', 'UDP')),
            host TEXT NOT NULL,
            port INTEGER NOT NULL,
            content TEXT NOT NULL,
            sender_info TEXT,
            session_id TEXT,
            format_type INTEGER DEFAULT 0
        );
    )";

    QString createIndexTimestamp = "CREATE INDEX IF NOT EXISTS idx_timestamp ON messages(timestamp);";
    QString createIndexDirection = "CREATE INDEX IF NOT EXISTS idx_direction ON messages(direction);";
    QString createIndexSession = "CREATE INDEX IF NOT EXISTS idx_session ON messages(session_id);";
    QString createIndexFormat = "CREATE INDEX IF NOT EXISTS idx_format ON messages(format_type);";

    bool success = executeQuery(createMessagesTable) &&
                   executeQuery(createIndexTimestamp) &&
                   executeQuery(createIndexDirection) &&
                   executeQuery(createIndexSession) &&
                   executeQuery(createIndexFormat);

    // Check if format_type column exists before adding it
    QSqlQuery checkColumnQuery(db);
    bool columnExists = false;
    if (checkColumnQuery.exec("PRAGMA table_info(messages);")) {
        while (checkColumnQuery.next()) {
            if (checkColumnQuery.value("name").toString() == "format_type") {
                columnExists = true;
                break;
            }
        }
    }

    // Only add column if it doesn't already exist
    if (!columnExists) {
        QString addFormatColumn = "ALTER TABLE messages ADD COLUMN format_type INTEGER DEFAULT 0;";
        executeQuery(addFormatColumn);
    }

    return success;
}

bool MessageHistoryManager::executeQuery(const QString &query)
{
    QSqlQuery q(db);
    if (!q.exec(query)) {
        qCritical() << "Query failed:" << query << "Error:" << q.lastError().text();
        return false;
    }
    return true;
}

bool MessageHistoryManager::saveMessage(const QString &direction, const QString &protocol,
                                       const QString &host, int port, const DataMessage &message,
                                       const QString &senderInfo)
{
    QMutexLocker locker(&dbMutex);

    if (!db.isOpen()) {
        qWarning() << "Database not open, cannot save message";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO messages (direction, protocol, host, port, content, sender_info, session_id, format_type) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(direction);
    query.addBindValue(protocol);
    query.addBindValue(host);
    query.addBindValue(port);
    query.addBindValue(message.toDisplayString()); // Store display string for all formats
    query.addBindValue(senderInfo);
    query.addBindValue(currentSessionId);
    query.addBindValue(static_cast<int>(message.type)); // Store format type

    if (!query.exec()) {
        qCritical() << "Failed to save message:" << query.lastError().text();
        return false;
    }

    lastUpdateTime = QDateTime::currentDateTime();
    return true;
}

QList<QVariantMap> MessageHistoryManager::getMessages(const QString &filter,
                                                     const QDateTime &startDate,
                                                     const QDateTime &endDate,
                                                     int limit, int offset)
{
    QMutexLocker locker(&dbMutex);
    QList<QVariantMap> messages;

    if (!db.isOpen()) {
        qWarning() << "Database not open, cannot get messages";
        return messages;
    }

    QString queryStr = "SELECT id, timestamp, direction, protocol, host, port, content, sender_info, session_id "
                       "FROM messages WHERE 1=1";
    QStringList conditions;
    QVariantList params;

    // Enhanced filter parsing
    if (!filter.isEmpty()) {
        QStringList filterParts = filter.split(' ', QString::SkipEmptyParts);
        for (const QString &part : filterParts) {
            if (part.startsWith("direction:")) {
                conditions << "direction = ?";
                params << part.mid(10); // Remove "direction:"
            } else if (part.startsWith("protocol:")) {
                conditions << "protocol = ?";
                params << part.mid(9); // Remove "protocol:"
            } else if (part.startsWith("host:")) {
                conditions << "host LIKE ?";
                params << "%" + part.mid(5) + "%"; // Remove "host:"
            } else {
                // General text search
                conditions << "(content LIKE ? OR sender_info LIKE ? OR host LIKE ?)";
                QString likeFilter = "%" + part + "%";
                params << likeFilter << likeFilter << likeFilter;
            }
        }
    }

    if (!conditions.isEmpty()) {
        queryStr += " AND (" + conditions.join(" AND ") + ")";
    }

    if (startDate.isValid()) {
        queryStr += " AND timestamp >= ?";
        params << startDate.toString(Qt::ISODate);
    }

    if (endDate.isValid()) {
        queryStr += " AND timestamp <= ?";
        params << endDate.toString(Qt::ISODate);
    }

    queryStr += " ORDER BY timestamp DESC LIMIT ? OFFSET ?";
    params << limit << offset;

    QSqlQuery query(db);
    query.prepare(queryStr);
    
    for (const QVariant &param : params) {
        query.addBindValue(param);
    }

    if (!query.exec()) {
        qCritical() << "Failed to get messages:" << query.lastError().text();
        qCritical() << "Query:" << queryStr;
        return messages;
    }

    messages.reserve(limit); // Pre-allocate for better performance
    while (query.next()) {
        QVariantMap message;
        message["id"] = query.value(0);
        message["timestamp"] = query.value(1);
        message["direction"] = query.value(2);
        message["protocol"] = query.value(3);
        message["host"] = query.value(4);
        message["port"] = query.value(5);
        message["content"] = query.value(6);
        message["sender_info"] = query.value(7);
        message["session_id"] = query.value(8);
        message["format_type"] = query.value(9);
        messages.append(message);
    }

    return messages;
}

bool MessageHistoryManager::deleteOldMessages(int daysToKeep)
{
    QDateTime cutoff = QDateTime::currentDateTime().addDays(-daysToKeep);
    QSqlQuery query(db);
    query.prepare("DELETE FROM messages WHERE timestamp < ?");
    query.addBindValue(cutoff.toString(Qt::ISODate));

    if (!query.exec()) {
        qCritical() << "Failed to delete old messages:" << query.lastError().text();
        return false;
    }
    return true;
}

bool MessageHistoryManager::exportMessages(const QString &filePath, const QString &format)
{
    // Implementation for export - simplified for now
    QList<QVariantMap> messages = getMessages(QString(), QDateTime(), QDateTime(), 10000, 0);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Failed to open export file:" << filePath;
        return false;
    }

    QTextStream out(&file);
    if (format == "json") {
        QJsonArray array;
        for (const QVariantMap &msg : messages) {
            QJsonObject obj;
            for (auto it = msg.begin(); it != msg.end(); ++it) {
                obj[it.key()] = QJsonValue::fromVariant(it.value());
            }
            array.append(obj);
        }
        QJsonDocument doc(array);
        out << doc.toJson(QJsonDocument::Indented);
    } else {
        // CSV format
        out << "ID,Timestamp,Direction,Protocol,Host,Port,Content,Sender Info,Session ID,Format Type\n";
        for (const QVariantMap &msg : messages) {
            out << msg["id"].toString() << ",";
            out << msg["timestamp"].toString() << ",";
            out << msg["direction"].toString() << ",";
            out << msg["protocol"].toString() << ",";
            out << msg["host"].toString() << ",";
            out << msg["port"].toString() << ",";
            out << "\"" << msg["content"].toString().replace("\"", "\"\"") << "\",";
            out << "\"" << msg["sender_info"].toString().replace("\"", "\"\"") << "\",";
            out << msg["session_id"].toString() << ",";
            out << msg["format_type"].toString() << "\n";
        }
    }

    file.close();
    return true;
}

void MessageHistoryManager::startNewSession()
{
    currentSessionId = generateSessionId();
}

QString MessageHistoryManager::generateSessionId()
{
    return QUuid::createUuid().toString().remove('{').remove('}');
}

bool MessageHistoryManager::clearAllMessages()
{
    QMutexLocker locker(&dbMutex);
    
    if (!db.isOpen()) {
        qWarning() << "Database not open, cannot clear messages";
        return false;
    }

    QSqlQuery query(db);
    if (!query.exec("DELETE FROM messages")) {
        qCritical() << "Failed to clear messages:" << query.lastError().text();
        return false;
    }
    
    lastUpdateTime = QDateTime::currentDateTime();
    return true;
}

int MessageHistoryManager::getMessageCount(const QString &filter)
{
    QMutexLocker locker(&dbMutex);
    
    if (!db.isOpen()) {
        return 0;
    }

    QString queryStr = "SELECT COUNT(*) FROM messages";
    if (!filter.isEmpty()) {
        queryStr += " WHERE (content LIKE ? OR sender_info LIKE ? OR host LIKE ?)";
    }

    QSqlQuery query(db);
    query.prepare(queryStr);
    
    if (!filter.isEmpty()) {
        QString likeFilter = "%" + filter + "%";
        query.addBindValue(likeFilter);
        query.addBindValue(likeFilter);
        query.addBindValue(likeFilter);
    }

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

QDateTime MessageHistoryManager::getLastMessageTime()
{
    return lastUpdateTime;
}

bool MessageHistoryManager::optimizeDatabase()
{
    QMutexLocker locker(&dbMutex);
    
    if (!db.isOpen()) {
        return false;
    }

    QSqlQuery query(db);
    return query.exec("VACUUM") && query.exec("ANALYZE");
}
