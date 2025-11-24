#include "gui.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QJsonDocument>

JsonSenderGUI::JsonSenderGUI() {
    setWindowTitle("JSON Sender GUI");
    resize(500, 400);
    
    auto *layout = new QVBoxLayout(this);
    
    // Connection settings
    auto *connLayout = new QHBoxLayout();
    protocolCombo = new QComboBox();
    protocolCombo->addItems({"tcp", "udp"});
    hostEdit = new QLineEdit("127.0.0.1");
    portEdit = new QLineEdit("5000");
    connectBtn = new QPushButton("Connect");
    
    connLayout->addWidget(new QLabel("Protocol:"));
    connLayout->addWidget(protocolCombo);
    connLayout->addWidget(new QLabel("Host:"));
    connLayout->addWidget(hostEdit);
    connLayout->addWidget(new QLabel("Port:"));
    connLayout->addWidget(portEdit);
    connLayout->addWidget(connectBtn);
    
    // JSON input
    jsonEdit = new QTextEdit();
    jsonEdit->setPlainText(R"({"type":"hello","from":"gui","value":42})");
    sendBtn = new QPushButton("Send JSON");
    sendBtn->setEnabled(false);
    
    // Log output
    logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    
    layout->addLayout(connLayout);
    layout->addWidget(new QLabel("JSON Message:"));
    layout->addWidget(jsonEdit);
    layout->addWidget(sendBtn);
    layout->addWidget(new QLabel("Log:"));
    layout->addWidget(logEdit);
    
    connect(connectBtn, &QPushButton::clicked, this, &JsonSenderGUI::onConnect);
    connect(sendBtn, &QPushButton::clicked, this, &JsonSenderGUI::onSend);
}

void JsonSenderGUI::onConnect() {
    QString proto = protocolCombo->currentText();
    QString host = hostEdit->text();
    int port = portEdit->text().toInt();
    
    bool connected = false;
    if (proto == "tcp") {
        connected = sender.connectTcp(host, port);
    } else {
        connected = sender.connectUdp(host, port);
    }
    
    if (connected) {
        logEdit->append(QString("✅ Connected to %1:%2 via %3").arg(host).arg(port).arg(proto.toUpper()));
        sendBtn->setEnabled(true);
        connectBtn->setText("Disconnect");
    } else {
        logEdit->append("❌ Connection failed");
    }
}

void JsonSenderGUI::onSend() {
    QString jsonText = jsonEdit->toPlainText();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        logEdit->append("❌ Invalid JSON: " + error.errorString());
        return;
    }
    
    sender.sendJson(doc);
    logEdit->append("→ Sent: " + jsonText);
}