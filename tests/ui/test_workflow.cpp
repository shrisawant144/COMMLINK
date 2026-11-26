#include "commlink/ui/mainwindow.h"
#include <QtTest>
#include <QTemporaryDir>
#include <QJsonArray>

class WorkflowTest : public QObject {
    Q_OBJECT
private slots:
    void saveReopenSendInspectReplay() {
        QTcpServer peer; QVERIFY(peer.listen(QHostAddress::LocalHost));
        {
            MainWindow window;
            auto *connection = window.findChild<ConnectionPanel *>();
            connection->setProtocol("TCP"); connection->setHost("${HOST}"); connection->setPort(peer.serverPort());
            window.findChild<MessagePanel *>()->setDataFormat("Text");
            window.findChild<MessagePanel *>()->setMessage("hello");
            auto *workspace = window.findChild<WorkbenchPanel *>();
            workspace->setOptions({{"framing", 3}, {"variables", QJsonObject{{"HOST", "127.0.0.1"}}}});
            workspace->saveRequested("Loopback");
        }
        MainWindow reopened;
        auto *composer = reopened.findChild<MessagePanel *>();
        QCOMPARE(composer->getMessage(), QString("hello"));
        QCOMPARE(reopened.findChild<ConnectionPanel *>()->getHost(), QString("${HOST}"));
        QVERIFY(QMetaObject::invokeMethod(&reopened, "onConnectRequested"));
        QTRY_VERIFY(peer.hasPendingConnections()); QTRY_VERIFY(reopened.findChild<TcpClient *>()->isConnected());
        auto *socket = peer.nextPendingConnection();
        QVERIFY(QMetaObject::invokeMethod(&reopened, "onSendRequested"));
        StreamFramer framing; framing.mode = StreamFramer::Length32;
        QTRY_COMPARE(socket->bytesAvailable(), qint64(9));
        QCOMPARE(socket->readAll(), framing.encode("hello"));
        socket->write(framing.encode("echo")); socket->flush();
        auto *traffic = reopened.findChild<TrafficInspector *>();
        QTRY_COMPARE(traffic->records().size(), 2);
        QCOMPARE(QByteArray::fromBase64(traffic->records().last().toObject()["payloadBase64"].toString().toLatin1()), QByteArray("echo"));
        traffic->replayRequested("echo", int(DataFormatType::TEXT));
        QCOMPARE(composer->getMessage(), QString("echo"));
        QVERIFY(QMetaObject::invokeMethod(&reopened, "onSendRequested"));
        QTRY_COMPARE(socket->bytesAvailable(), qint64(8)); QCOMPARE(socket->readAll(), framing.encode("echo"));
        QVERIFY(QMetaObject::invokeMethod(&reopened, "onDisconnectRequested"));
    }
};
int main(int argc, char **argv) {
    QTemporaryDir state; qputenv("XDG_CONFIG_HOME", state.path().toUtf8()); qputenv("XDG_DATA_HOME", state.path().toUtf8());
    QSettings::setDefaultFormat(QSettings::IniFormat); QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, state.path());
    QApplication app(argc, argv); app.setApplicationName("CommLinkWorkflowTest");
    WorkflowTest test; return QTest::qExec(&test, argc, argv);
}
#include "test_workflow.moc"
