#include "commlink/ui/mainwindow.h"

#include <QJsonArray>
#include <QTemporaryDir>
#include <QtTest>

class WorkflowTest : public QObject {
    Q_OBJECT
private slots:
    void init() {
        QSettings("CommLink", "ModularGUI").clear();
    }
    void workspaceIdentitySurvivesRestart() {
        {
            MainWindow window;
            window.findChild<WorkbenchPanel*>()->saveRequested("Named workspace");
        }
        MainWindow reopened;
        QCOMPARE(reopened.findChild<WorkbenchPanel*>()->profileName(), QString("Named workspace"));
    }
    void workspaceConversationSurvivesRestart() {
        QTcpServer peer;
        QVERIFY(peer.listen(QHostAddress::LocalHost));
        QJsonArray original;
        WorkspaceStore store;
        QVERIFY(
            store.save("Empty session", {{"sessionId", MessageHistoryManager::generateSessionId()},
                                         {"workspaceName", "Empty session"}}));
        {
            MainWindow window;
            auto* connection = window.findChild<ConnectionPanel*>();
            connection->setProtocol("TCP");
            connection->setHost("127.0.0.1");
            connection->setPort(peer.serverPort());
            window.findChild<MessagePanel*>()->setDataFormat("Hex");
            window.findChild<MessagePanel*>()->setMessage("00 ff 80");
            window.findChild<WorkbenchPanel*>()->setOptions({{"framing", 3}});
            window.findChild<WorkbenchPanel*>()->saveRequested("Saved conversation");
            QVERIFY(QMetaObject::invokeMethod(&window, "onConnectRequested"));
            QTRY_VERIFY(peer.hasPendingConnections());
            QTRY_VERIFY(window.findChild<TcpClient*>()->isConnected());
            auto* socket = peer.nextPendingConnection();
            QVERIFY(QMetaObject::invokeMethod(&window, "onSendRequested"));
            QTRY_COMPARE(socket->bytesAvailable(), qint64(7));
            const auto bytes = socket->readAll();
            socket->write(bytes);
            socket->flush();
            auto* traffic = window.findChild<TrafficInspector*>();
            QTRY_COMPARE(traffic->records().size(), 2);
            original = traffic->records();
            QVERIFY(QMetaObject::invokeMethod(&window, "onDisconnectRequested"));
        }
        MainWindow reopened;
        const auto restored = reopened.findChild<TrafficInspector*>()->records();
        QCOMPARE(reopened.findChild<WorkbenchPanel*>()->profileName(),
                 QString("Saved conversation"));
        QCOMPARE(restored, original);
        // Opening a different named session must not merge its traffic into this one.
        reopened.findChild<WorkbenchPanel*>()->openRequested("Empty session");
        QVERIFY(reopened.findChild<TrafficInspector*>()->records().isEmpty());
    }
    void httpTimelineMatchesTransmittedBody() {
        QTcpServer peer;
        QVERIFY(peer.listen(QHostAddress::LocalHost));
        MainWindow window;
        auto* connection = window.findChild<ConnectionPanel*>();
        connection->setProtocol("HTTP");
        connection->setHost(QString("http://127.0.0.1:%1/").arg(peer.serverPort()));
        connection->setHttpMethod("GET");
        window.findChild<MessagePanel*>()->setDataFormat("JSON");
        window.findChild<MessagePanel*>()->setMessage(
            "ignored by GET, including invalid JSON and ${UNDEFINED}");
        QVERIFY(QMetaObject::invokeMethod(&window, "onSendRequested"));
        QTRY_VERIFY(peer.hasPendingConnections());
        const auto records = window.findChild<TrafficInspector*>()->records();
        QCOMPARE(records.size(), 1);
        QCOMPARE(QByteArray::fromBase64(
                     records.first().toObject()["payloadBase64"].toString().toLatin1()),
                 QByteArray());
        window.findChild<HttpClient*>()->disconnect();
    }
    void replayIgnoresEarlierHttpResponse_data() {
        QTest::addColumn<int>("earlierStatus");
        QTest::newRow("earlier success") << 200;
        QTest::newRow("earlier HTTP error") << 404;
    }
    void replayIgnoresEarlierHttpResponse() {
        QFETCH(int, earlierStatus);
        QTcpServer peer;
        QVERIFY(peer.listen(QHostAddress::LocalHost));
        MainWindow window;
        auto* connection = window.findChild<ConnectionPanel*>();
        connection->setProtocol("HTTP");
        connection->setHost(QString("http://127.0.0.1:%1/").arg(peer.serverPort()));
        connection->setHttpMethod("GET");
        auto* composer = window.findChild<MessagePanel*>();
        composer->setDataFormat("Text");
        composer->setMessage("request");
        QVERIFY(QMetaObject::invokeMethod(&window, "onSendRequested"));
        QTRY_VERIFY(peer.hasPendingConnections());
        auto* earlier = peer.nextPendingConnection();
        QSignalSpy received(window.findChild<HttpClient*>(), &HttpClient::responseReceived);
        window.findChild<WorkbenchPanel*>()->replayRequested(
            {{"count", 1}, {"assertion", 1}, {"expected", "expected"}});
        QTRY_VERIFY(peer.hasPendingConnections());
        auto* current = peer.nextPendingConnection();
        const QByteArray response =
            "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 8\r\n\r\nexpected";
        earlier->write(earlierStatus == 404
                           ? QByteArray(response).replace("200 OK", "404 Not Found")
                           : response);
        earlier->flush();
        QTRY_COMPARE(received.count(), 1);
        QVERIFY(!composer->isEnabled());
        current->write(response);
        current->flush();
        QTRY_VERIFY(composer->isEnabled());
        window.findChild<HttpClient*>()->disconnect();
    }
    void saveReopenSendInspectReplay() {
        QTcpServer peer;
        QVERIFY(peer.listen(QHostAddress::LocalHost));
        {
            MainWindow window;
            auto* connection = window.findChild<ConnectionPanel*>();
            connection->setProtocol("TCP");
            connection->setHost("${HOST}");
            connection->setPort(peer.serverPort());
            window.findChild<MessagePanel*>()->setDataFormat("Text");
            window.findChild<MessagePanel*>()->setMessage("hello");
            auto* workspace = window.findChild<WorkbenchPanel*>();
            workspace->setOptions(
                {{"framing", 3}, {"variables", QJsonObject{{"HOST", "127.0.0.1"}}}});
            workspace->saveRequested("Loopback");
        }
        MainWindow reopened;
        auto* composer = reopened.findChild<MessagePanel*>();
        QCOMPARE(composer->getMessage(), QString("hello"));
        QCOMPARE(reopened.findChild<ConnectionPanel*>()->getHost(), QString("${HOST}"));
        QVERIFY(QMetaObject::invokeMethod(&reopened, "onConnectRequested"));
        QTRY_VERIFY(peer.hasPendingConnections());
        QTRY_VERIFY(reopened.findChild<TcpClient*>()->isConnected());
        auto* socket = peer.nextPendingConnection();
        QVERIFY(QMetaObject::invokeMethod(&reopened, "onSendRequested"));
        StreamFramer framing;
        framing.mode = StreamFramer::Length32;
        QTRY_COMPARE(socket->bytesAvailable(), qint64(9));
        QCOMPARE(socket->readAll(), framing.encode("hello"));
        socket->write(framing.encode("echo"));
        socket->flush();
        auto* traffic = reopened.findChild<TrafficInspector*>();
        QTRY_COMPARE(traffic->records().size(), 2);
        QCOMPARE(traffic->records().first().toObject()["endpoint"].toString(),
                 QString("127.0.0.1:%1").arg(peer.serverPort()));
        QCOMPARE(QByteArray::fromBase64(
                     traffic->records().last().toObject()["payloadBase64"].toString().toLatin1()),
                 QByteArray("echo"));
        traffic->replayRequested("echo", int(DataFormatType::TEXT));
        QCOMPARE(composer->getMessage(), QString("echo"));
        QVERIFY(QMetaObject::invokeMethod(&reopened, "onSendRequested"));
        QTRY_COMPARE(socket->bytesAvailable(), qint64(8));
        QCOMPARE(socket->readAll(), framing.encode("echo"));
        connect(socket, &QTcpSocket::readyRead, socket, [socket] {
            socket->write(socket->readAll());
            socket->flush();
        });
        reopened.findChild<WorkbenchPanel*>()->replayRequested(
            {{"count", 2}, {"intervalMs", 10}, {"assertion", 1}, {"expected", "echo"}});
        QTRY_COMPARE(traffic->records().size(), 7);
        QTRY_VERIFY(composer->isEnabled());
        QVERIFY(QMetaObject::invokeMethod(&reopened, "onDisconnectRequested"));
    }
};
int main(int argc, char** argv) {
    QTemporaryDir state;
    qputenv("XDG_CONFIG_HOME", state.path().toUtf8());
    qputenv("XDG_DATA_HOME", state.path().toUtf8());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, state.path());
    QApplication app(argc, argv);
    app.setApplicationName("CommLinkWorkflowTest");
    WorkflowTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "test_workflow.moc"
