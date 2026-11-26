#include "commlink/core/streamframer.h"
#include "commlink/network/tcpclient.h"
#include "commlink/network/tcpserver.h"

#include <QtTest>

class TransportTest : public QObject {
    Q_OBJECT
private slots:
    void bytesAndFraming() {
        const QByteArray bytes = QByteArray::fromHex("00ff0180");
        for (auto format : {DataFormatType::HEX, DataFormatType::BINARY, DataFormatType::JSON})
            QCOMPARE(DataMessage::deserialize(bytes, format).serialize(), bytes);
        QCOMPARE(DataMessage(DataFormatType::HEX, bytes).serialize(), bytes);
        QCOMPARE(DataMessage(DataFormatType::XML, QString("<x/>")).serialize(), QByteArray("<x/>"));
        QVERIFY(!DataMessage::validateInput("f", DataFormatType::HEX));
        QVERIFY(!DataMessage::validateInput("<x>", DataFormatType::XML));
        for (auto mode : {StreamFramer::Newline, StreamFramer::Delimiter, StreamFramer::Length32}) {
            StreamFramer framer;
            framer.mode = mode;
            framer.delimiter = "\r\n";
            const QByteArray first = framer.encode("first");
            QVERIFY(framer.feed(first.left(2)).isEmpty());
            const auto frames = framer.feed(first.mid(2) + framer.encode("second"));
            QCOMPARE(frames.size(), 2);
            QCOMPARE(frames[0].payload, QByteArray("first"));
            QCOMPARE(frames[0].wire, first);
            QCOMPARE(frames[1].payload, QByteArray("second"));
        }
        StreamFramer oversized;
        oversized.mode = StreamFramer::Length32;
        QVERIFY(oversized.feed(QByteArray::fromHex("7fffffff")).isEmpty());
        QVERIFY(!oversized.error.isEmpty());
    }
    void boundedFrameBatchesYieldToOtherEvents() {
        StreamFramer framer;
        framer.mode = StreamFramer::Newline;
        const QByteArray burst = QByteArray("x\n").repeated(1000);
        int decoded = framer.feed(burst).size();
        QCOMPARE(decoded, StreamFramer::FrameBatchSize);
        while (decoded < 1000) {
            const auto batch = framer.feed({});
            QVERIFY(!batch.isEmpty());
            QVERIFY(batch.size() <= StreamFramer::FrameBatchSize);
            decoded += batch.size();
        }
        TcpServer server;
        server.setFraming(StreamFramer::Newline);
        server.setFormat(DataFormatType::TEXT);
        QVERIFY(server.startServer(0, QHostAddress::LocalHost));
        QTcpSocket peer;
        peer.connectToHost(QHostAddress::LocalHost, server.localPort());
        QTRY_COMPARE(peer.state(), QAbstractSocket::ConnectedState);
        int received = 0;
        bool callbackRan = false, yielded = false;
        connect(&server, &TcpServer::messageReceived, &server, [&] {
            if (++received == 1)
                QTimer::singleShot(0, &server, [&] { callbackRan = true; });
            if (received == 1000)
                yielded = callbackRan;
        });
        peer.write(burst);
        peer.flush();
        QTRY_COMPARE(received, 1000);
        QVERIFY(yielded);
        peer.write(burst);
        peer.disconnectFromHost();
        QTRY_COMPARE(received, 2000);
        server.stopServer();
        QTcpServer remote;
        QVERIFY(remote.listen(QHostAddress::LocalHost));
        TcpClient client;
        client.setFormat(DataFormatType::TEXT);
        client.setFraming(StreamFramer::Newline);
        client.connectToHost("127.0.0.1", remote.serverPort());
        QTRY_VERIFY(remote.hasPendingConnections());
        auto* socket = remote.nextPendingConnection();
        int clientReceived = 0;
        bool clientYielded = false, clientCallback = false;
        connect(&client, &TcpClient::messageReceived, &client, [&] {
            if (++clientReceived == 1)
                QTimer::singleShot(0, &client, [&] { clientCallback = true; });
            if (clientReceived == 1000)
                clientYielded = clientCallback;
        });
        socket->write(burst);
        socket->flush();
        QTRY_COMPARE(clientReceived, 1000);
        QVERIFY(clientYielded);
        socket->write(burst);
        socket->disconnectFromHost();
        QTRY_COMPARE(clientReceived, 2000);
    }
    void fragmentedLoopbackAndRouting() {
        TcpServer server;
        server.setFormat(DataFormatType::TEXT);
        server.setFraming(StreamFramer::Newline);
        QVERIFY(server.startServer(0, QHostAddress::LocalHost));
        QSignalSpy received(&server, &TcpServer::messageReceived);
        QSignalSpy joined(&server, &TcpServer::clientConnected);
        QTcpSocket first, second;
        first.connectToHost(QHostAddress::LocalHost, server.localPort());
        second.connectToHost(QHostAddress::LocalHost, server.localPort());
        QTRY_COMPARE(joined.count(), 2);
        first.write("hel");
        first.flush();
        QTest::qWait(20);
        QCOMPARE(received.count(), 0);
        first.write("lo\nworld\n");
        first.flush();
        QTRY_COMPARE(received.count(), 2);
        const auto msg = qvariant_cast<DataMessage>(received[0][0]);
        QCOMPARE(msg.serialize(), QByteArray("hello"));
        QCOMPARE(msg.wireBytes, QByteArray("hello\n"));
        const QString peer = QString("127.0.0.1:%1").arg(first.localPort());
        QVERIFY(server.findClientByAddress(peer));
        server.sendToClient(server.findClientByAddress(peer),
                            DataMessage(DataFormatType::TEXT, "one"));
        QTRY_COMPARE(first.bytesAvailable(), qint64(4));
        QCOMPARE(first.readAll(), QByteArray("one\n"));
        QCOMPARE(second.bytesAvailable(), qint64(0));
        server.sendToAll(DataMessage(DataFormatType::TEXT, "all"));
        QTRY_COMPARE(first.bytesAvailable(), qint64(4));
        QTRY_COMPARE(second.bytesAvailable(), qint64(4));
        QCOMPARE(second.readAll(), QByteArray("all\n"));
        first.disconnectFromHost();
        second.disconnectFromHost();
        server.stopServer();
    }
    void clientFramingAndDisconnect() {
        QTcpServer peer;
        QVERIFY(peer.listen(QHostAddress::LocalHost));
        TcpClient client;
        client.setFormat(DataFormatType::HEX);
        client.setFraming(StreamFramer::Length32);
        QSignalSpy connected(&client, &TcpClient::connected),
            received(&client, &TcpClient::messageReceived);
        QSignalSpy disconnected(&client, &TcpClient::disconnected);
        client.connectToHost("127.0.0.1", peer.serverPort());
        QTRY_COMPARE(connected.count(), 1);
        QTRY_VERIFY(peer.hasPendingConnections());
        auto* socket = peer.nextPendingConnection();
        StreamFramer encoder;
        encoder.mode = StreamFramer::Length32;
        const QByteArray wire = encoder.encode(QByteArray::fromHex("00ff"));
        socket->write(wire.left(3));
        socket->flush();
        QTest::qWait(20);
        QCOMPARE(received.count(), 0);
        socket->write(wire.mid(3));
        socket->flush();
        QTRY_COMPARE(received.count(), 1);
        QCOMPARE(qvariant_cast<DataMessage>(received[0][0]).serialize(),
                 QByteArray::fromHex("00ff"));
        client.sendMessage(DataMessage(DataFormatType::HEX, QByteArray::fromHex("80ff")));
        QTRY_COMPARE(socket->bytesAvailable(), qint64(6));
        QCOMPARE(socket->readAll(), encoder.encode(QByteArray::fromHex("80ff")));
        socket->disconnectFromHost();
        QTRY_COMPARE(disconnected.count(), 1);
        QVERIFY(!client.isConnected());
    }
};
QTEST_GUILESS_MAIN(TransportTest)
#include "test_transport.moc"
