#include "commlink/core/workspacestore.h"
#include "commlink/core/messagehistorymanager.h"
#include <QtTest>
#include <QTemporaryDir>
#include <QSqlQuery>

class WorkspaceTest : public QObject {
    Q_OBJECT
private slots:
    void roundTripAndVariables() {
        QTemporaryDir dir; WorkspaceStore store(dir.path()); QString error;
        QJsonObject state{{"host", "${HOST}"}, {"payload", "hello"}, {"options", QJsonObject{{"framing", 3}}}};
        QVERIFY(store.save("Local TCP", state, &error)); QVERIFY(error.isEmpty());
        QCOMPARE(store.names(), QStringList{"Local TCP"});
        const auto opened = store.load("Local TCP", &error);
        QCOMPARE(opened["payload"].toString(), QString("hello")); QCOMPARE(opened["options"].toObject()["framing"].toInt(), 3);
        QVERIFY(!store.save("../escape", state, &error)); QVERIFY(!error.isEmpty());
        QCOMPARE(WorkspaceStore::expand("tcp://${HOST}", {{"HOST", "127.0.0.1"}}, &error), QString("tcp://127.0.0.1"));
        WorkspaceStore::expand("${MISSING}", {}, &error); QVERIFY(!error.isEmpty());
    }
    void migrateAndPreserveBytes() {
        QTemporaryDir dir; const auto path = dir.filePath("history.db");
        {
            auto db = QSqlDatabase::addDatabase("QSQLITE", "legacy-test"); db.setDatabaseName(path); QVERIFY(db.open());
            QSqlQuery query(db);
            QVERIFY(query.exec("CREATE TABLE messages (id INTEGER PRIMARY KEY, timestamp TEXT, direction TEXT, protocol TEXT CHECK(protocol IN ('TCP','UDP')), host TEXT, port INTEGER, content TEXT, sender_info TEXT, session_id TEXT)"));
            QVERIFY(query.exec("INSERT INTO messages VALUES(1,'2026-01-01','received','TCP','localhost',12,'saved','peer','session')"));
        }
        QSqlDatabase::removeDatabase("legacy-test");
        MessageHistoryManager history; QVERIFY(history.initializeDatabase(path));
        QCOMPARE(history.getMessageCount(), 1);
        auto msg = DataMessage::deserialize(QByteArray::fromHex("00ff80"), DataFormatType::HEX); msg.wireBytes = QByteArray::fromHex("0000000300ff80");
        QVERIFY(history.saveMessage("sent", "WebSocket", "localhost", 12, msg));
        const auto rows = history.getMessages(); QCOMPARE(rows.size(), 2);
        QCOMPARE(rows[0]["raw_payload"].toByteArray(), msg.serialize()); QCOMPARE(rows[0]["raw_wire"].toByteArray(), msg.wireBytes);
        QCOMPARE(rows[1]["content"].toString(), QString("saved"));
    }
};
QTEST_GUILESS_MAIN(WorkspaceTest)
#include "test_workspace.moc"
