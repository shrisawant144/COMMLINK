#include "sender.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    Sender sender;
    
    // Test TCP connection to localhost
    qInfo() << "Testing TCP connection...";
    bool tcpResult = sender.connectTcp("127.0.0.1", 12345);
    qInfo() << "TCP connection result:" << tcpResult;
    
    // Test UDP configuration
    qInfo() << "Testing UDP configuration...";
    bool udpResult = sender.connectUdp("127.0.0.1", 12346);
    qInfo() << "UDP configuration result:" << udpResult;
    
    // Test JSON creation and validation
    QJsonObject testObj;
    testObj["type"] = "test";
    testObj["message"] = "robustness check";
    testObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument testDoc(testObj);
    qInfo() << "Test JSON:" << testDoc.toJson(QJsonDocument::Compact);
    
    // Test receiver functionality
    qInfo() << "Testing TCP receiver...";
    bool receiverResult = sender.startTcpReceiver(12347);
    qInfo() << "TCP receiver result:" << receiverResult;
    
    if (receiverResult) {
        sender.stopReceiver();
        qInfo() << "Receiver stopped successfully";
    }
    
    qInfo() << "All tests completed";
    return 0;
}
