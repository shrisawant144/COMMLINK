#include "commlink/network/httpclient.h"
#include "commlink/network/httpserver.h"
#include "commlink/network/udpclient.h"
#include "commlink/network/udpserver.h"
#include "commlink/network/websocketclient.h"
#include "commlink/network/websocketserver.h"

#include <QtTest>

class ProtocolTest : public QObject {
    Q_OBJECT
private slots:
    void httpInspectionAndTimeout() {
        QTcpServer peer;
        QVERIFY(peer.listen(QHostAddress::LocalHost));
        HttpClient client;
        client.addHeader("X-Test", "custom");
        client.addHeader("Authorization", "Bearer example");
        QSignalSpy metadata(&client, &HttpClient::responseInspected),
            received(&client, &HttpClient::responseReceived),
            errors(&client, &HttpClient::errorOccurred);
        client.sendRequest(QString("http://127.0.0.1:%1/x").arg(peer.serverPort()),
                           HttpClient::GET);
        QTRY_VERIFY(peer.hasPendingConnections());
        auto* socket = peer.nextPendingConnection();
        QTRY_VERIFY(socket->bytesAvailable() > 0);
        const auto request = socket->readAll();
        QVERIFY(request.contains("X-Test: custom"));
        QVERIFY(request.contains("Authorization: Bearer example"));
        socket->write("HTTP/1.1 404 Not Found\r\nContent-Type: application/json\r\nContent-Length: "
                      "12\r\nX-Trace: abc\r\nConnection: close\r\n\r\n{\"ok\":false}");
        socket->flush();
        QTRY_COMPARE(metadata.count(), 1);
        QTRY_COMPARE(received.count(), 1);
        QCOMPARE(metadata[0][2].toInt(), 404);
        QVERIFY(metadata[0][3].toMap().contains("X-Trace"));
        QVERIFY(metadata[0][4].toLongLong() >= 0);
        QCOMPARE(qvariant_cast<DataMessage>(received[0][0]).serialize(),
                 QByteArray("{\"ok\":false}"));
        client.setTimeout(30);
        client.sendRequest(QString("http://127.0.0.1:%1/stall").arg(peer.serverPort()),
                           HttpClient::GET);
        QTRY_VERIFY(peer.hasPendingConnections());
        peer.nextPendingConnection();
        QTRY_COMPARE(metadata.count(), 2);
        QVERIFY(metadata[1][5].toString().contains("timed out"));
        QVERIFY(errors.count() >= 2);
    }
    void httpFragmentationAndInvalidRequests() {
        HttpServer server;
        QVERIFY(server.startServer(0, QHostAddress::LocalHost));
        QSignalSpy received(&server, &HttpServer::messageReceived);
        QTcpSocket socket;
        socket.connectToHost(QHostAddress::LocalHost, server.localPort());
        QTRY_COMPARE(socket.state(), QAbstractSocket::ConnectedState);
        socket.write("POST /x HTTP/1.1\r\ncontent-type: text/plain\r\ncontent-length: 5\r\n\r\nhe");
        socket.flush();
        QTest::qWait(20);
        QCOMPARE(received.count(), 0);
        socket.write("llo");
        socket.flush();
        QTRY_COMPARE(received.count(), 1);
        QCOMPARE(qvariant_cast<DataMessage>(received[0][0]).serialize(), QByteArray("hello"));
        QTRY_VERIFY(socket.bytesAvailable() > 0);
        QVERIFY(socket.readAll().startsWith("HTTP/1.1 200"));
        auto* serverPeer =
            server.findClientByAddress(QString("127.0.0.1:%1").arg(socket.localPort()));
        QVERIFY(serverPeer);
        server.queueMessageForClient(serverPeer, DataMessage(DataFormatType::TEXT, "queued"));
        socket.write("GET / HTTP/1.1\r\nAccept: text/plain\r\n\r\n");
        socket.flush();
        QTRY_VERIFY(socket.bytesAvailable() > 0);
        QVERIFY(socket.readAll().endsWith("queued"));
        QSignalSpy queueErrors(&server, &HttpServer::errorOccurred);
        DataMessage large(DataFormatType::BINARY, QByteArray(4 * 1024 * 1024, 'x'));
        for (int i = 0; i < 10; ++i)
            server.queueMessageForClient(serverPeer, large);
        QCOMPARE(queueErrors.count(), 2);
        socket.write("POST /x HTTP/1.1\r\nContent-Length: -2\r\n\r\n");
        socket.flush();
        QTRY_VERIFY(socket.bytesAvailable() > 0);
        QVERIFY(socket.readAll().startsWith("HTTP/1.1 400"));
        QTcpSocket chunked;
        chunked.connectToHost(QHostAddress::LocalHost, server.localPort());
        QTRY_COMPARE(chunked.state(), QAbstractSocket::ConnectedState);
        chunked.write("POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n");
        chunked.flush();
        QTRY_VERIFY(chunked.bytesAvailable() > 0);
        QVERIFY(chunked.readAll().startsWith("HTTP/1.1 501"));
        server.stopServer();
    }
    void udpHostnameAndBinaryRoundTrip() {
        UdpServer server;
        server.setFormat(DataFormatType::HEX);
        QVERIFY(server.startServer(0, QHostAddress::LocalHost));
        UdpClient client;
        client.setFormat(DataFormatType::HEX);
        QSignalSpy discovered(&server, &UdpServer::peerDiscovered),
            received(&client, &UdpClient::messageReceived);
        connect(&server, &UdpServer::messageReceived, &server,
                [&server](const DataMessage& msg) { server.sendToAll(msg); });
        QVERIFY(client.connectToHost("127.0.0.1", server.localPort()));
        QTRY_VERIFY(client.isConnected());
        client.sendMessage(DataMessage(DataFormatType::HEX, QByteArray::fromHex("00ff80")));
        QTRY_COMPARE(received.count(), 1);
        QCOMPARE(discovered.count(), 1);
        QCOMPARE(qvariant_cast<DataMessage>(received[0][0]).serialize(),
                 QByteArray::fromHex("00ff80"));
        client.disconnect();
    }
    void websocketBinaryAndPong() {
        WebSocketServer server;
        QVERIFY(server.startServer(0, QHostAddress::LocalHost));
        WebSocketClient client;
        client.setFormat(DataFormatType::HEX);
        QSignalSpy connected(&client, &WebSocketClient::connected),
            received(&server, &WebSocketServer::messageReceived),
            diagnostics(&client, &WebSocketClient::diagnostic);
        client.connectToServer(QString("ws://127.0.0.1:%1").arg(server.localPort()));
        QTRY_COMPARE(connected.count(), 1);
        client.sendMessage(DataMessage(DataFormatType::HEX, QByteArray::fromHex("00ff")));
        QTRY_COMPARE(received.count(), 1);
        QCOMPARE(qvariant_cast<DataMessage>(received[0][0]).serialize(),
                 QByteArray::fromHex("00ff"));
        client.ping("test");
        QTRY_VERIFY(diagnostics.count() > 0);
        QVERIFY(diagnostics.last()[0].toString().contains("Pong"));
        client.disconnect();
        QTRY_VERIFY(!client.isConnected());
        server.stopServer();
    }
};
QTEST_GUILESS_MAIN(ProtocolTest)
#include "test_protocols.moc"
