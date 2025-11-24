#include "../include/sender.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qInfo() << "=== JsonSender Basic Test ===";
    
    Sender sender;
    
    // Test 1: UDP configuration (should always work)
    qInfo() << "Test 1: UDP Configuration";
    bool udpResult = sender.connectUdp("127.0.0.1", 12346);
    qInfo() << "UDP result:" << (udpResult ? "✅ SUCCESS" : "❌ FAILED");
    
    // Test 2: JSON creation
    qInfo() << "Test 2: JSON Creation";
    QJsonObject testObj;
    testObj["type"] = "test";
    testObj["message"] = "basic test";
    testObj["port"] = 12346;
    
    QJsonDocument testDoc(testObj);
    qInfo() << "Test JSON:" << testDoc.toJson(QJsonDocument::Compact);
    
    // Test 3: TCP receiver on different port
    qInfo() << "Test 3: TCP Receiver";
    bool receiverResult = sender.startTcpReceiver(12347);
    qInfo() << "TCP receiver result:" << (receiverResult ? "✅ SUCCESS" : "❌ FAILED");
    
    if (receiverResult) {
        sender.stopReceiver();
        qInfo() << "✅ Receiver stopped successfully";
    }
    
    qInfo() << "=== All Basic Tests Completed ===";
    return 0;
}