#include "commlink/ui/trafficinspector.h"

#include <QJsonObject>
#include <QtTest>

class TrafficTest : public QObject {
    Q_OBJECT
private slots:
    void originalBytesAndBoundedRetention() {
        TrafficInspector inspector;
        auto binary = DataMessage::deserialize(QByteArray::fromHex("00ff80"), DataFormatType::HEX);
        binary.wireBytes = QByteArray::fromHex("0000000300ff80");
        inspector.append(binary, "RX", "TCP", "127.0.0.1:123");
        const auto record = inspector.records().first().toObject();
        QCOMPARE(QByteArray::fromBase64(record["payloadBase64"].toString().toLatin1()),
                 binary.serialize());
        QCOMPARE(QByteArray::fromBase64(record["wireBase64"].toString().toLatin1()),
                 binary.wireBytes);
        for (int i = 0; i < 2200; ++i)
            inspector.append(binary, "RX", "TCP", QString::number(i));
        QCOMPARE(inspector.records().size(), 2000);
        QCOMPARE(inspector.records().last().toObject()["endpoint"].toString(), QString("2199"));
        inspector.clear();
        QVERIFY(inspector.records().isEmpty());
    }
};
QTEST_MAIN(TrafficTest)
#include "test_traffic.moc"
