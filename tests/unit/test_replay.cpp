#include "commlink/core/replayengine.h"

#include <QtTest>

class ReplayTest : public QObject {
    Q_OBJECT
private slots:
    void countsAndCancel() {
        ReplayEngine engine;
        QSignalSpy sends(&engine, &ReplayEngine::sendRequested),
            done(&engine, &ReplayEngine::finished);
        QVERIFY(engine.start({{"count", 3}, {"intervalMs", 10}}));
        QTRY_COMPARE(done.count(), 1);
        QCOMPARE(sends.count(), 3);
        QVERIFY(done[0][0].toBool());
        QVERIFY(engine.start({{"count", 100}, {"intervalMs", 10}}));
        engine.stop();
        const int before = sends.count();
        QTest::qWait(30);
        QCOMPARE(sends.count(), before);
    }
    void assertionsAndTimeout() {
        ReplayEngine engine;
        QSignalSpy done(&engine, &ReplayEngine::finished);
        connect(&engine, &ReplayEngine::sendRequested, &engine,
                [&engine] { engine.receive("{\"result\":{\"ok\":true}}"); });
        QVERIFY(engine.start({{"count", 2},
                              {"intervalMs", 10},
                              {"assertion", 3},
                              {"path", "result.ok"},
                              {"expected", "true"}}));
        QTRY_COMPARE(done.count(), 1);
        QVERIFY(done[0][0].toBool());
        QVERIFY(engine.start({{"assertion", 1}, {"expected", "missing"}}));
        QTRY_COMPARE(done.count(), 2);
        QVERIFY(!done[1][0].toBool());
        engine.disconnect();
        QSignalSpy timed(&engine, &ReplayEngine::finished);
        QVERIFY(engine.start({{"assertion", 1}, {"expected", "ok"}, {"timeoutMs", 20}}));
        QTRY_COMPARE(timed.count(), 1);
        QVERIFY(!timed[0][0].toBool());
        QVERIFY(timed[0][1].toString().contains("timed out"));
    }
    void binaryEquality() {
        ReplayEngine engine;
        QSignalSpy done(&engine, &ReplayEngine::finished);
        connect(&engine, &ReplayEngine::sendRequested, &engine,
                [&engine] { engine.receive(QByteArray::fromHex("00ff")); });
        QVERIFY(engine.start({{"assertion", 2}, {"expected", "00 ff"}}));
        QTRY_COMPARE(done.count(), 1);
        QVERIFY(done[0][0].toBool());
    }
};
QTEST_GUILESS_MAIN(ReplayTest)
#include "test_replay.moc"
