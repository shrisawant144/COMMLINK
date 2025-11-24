#include "../include/gui.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QIntValidator>
#include <QMessageBox>
#include <QDateTime>
#include <QThread>

JsonSenderGUI::JsonSenderGUI() {
    setWindowTitle("JSON Sender/Receiver");
    resize(700, 700);
    setupUI();
    setupValidators();
    updateConnectionState(false);
    updateReceiveState(false);
}

void JsonSenderGUI::setupUI()
{
    auto *layout = new QVBoxLayout(this);

    // Connection settings
    auto *connGroup = new QGroupBox("Connection Settings");
    auto *connLayout = new QHBoxLayout(connGroup);
    
    protocolCombo = new QComboBox();
    protocolCombo->addItems({"TCP", "UDP"});
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

    // Send section
    auto *sendGroup = new QGroupBox("Send JSON");
    auto *sendLayout = new QVBoxLayout(sendGroup);
    
    jsonEdit = new QTextEdit();
    jsonEdit->setPlainText(R"({"type":"hello","from":"gui","value":42})");
    jsonEdit->setMaximumHeight(100);
    
    sendBtn = new QPushButton("Send JSON");

    sendLayout->addWidget(new QLabel("JSON Message:"));
    sendLayout->addWidget(jsonEdit);
    sendLayout->addWidget(sendBtn);

    // Receive section
    auto *receiveGroup = new QGroupBox("Receive JSON");
    auto *receiveLayout = new QVBoxLayout(receiveGroup);
    auto *receiveBtnLayout = new QHBoxLayout();
    
    receivePortEdit = new QLineEdit("5001");
    startReceiveBtn = new QPushButton("Start Receiving");
    stopReceiveBtn = new QPushButton("Stop Receiving");

    receiveBtnLayout->addWidget(new QLabel("Listen Port:"));
    receiveBtnLayout->addWidget(receivePortEdit);
    receiveBtnLayout->addWidget(startReceiveBtn);
    receiveBtnLayout->addWidget(stopReceiveBtn);

    receivedEdit = new QTextEdit();
    receivedEdit->setReadOnly(true);
    receivedEdit->setMaximumHeight(150);

    receiveLayout->addLayout(receiveBtnLayout);
    receiveLayout->addWidget(new QLabel("Received Messages:"));
    receiveLayout->addWidget(receivedEdit);

    // Log section
    auto *logGroup = new QGroupBox("Activity Log");
    auto *logLayout = new QVBoxLayout(logGroup);
    logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    logLayout->addWidget(logEdit);

    layout->addWidget(connGroup);
    layout->addWidget(sendGroup);
    layout->addWidget(receiveGroup);
    layout->addWidget(logGroup);

    // Connect signals
    connect(connectBtn, &QPushButton::clicked, this, &JsonSenderGUI::onConnect);
    connect(sendBtn, &QPushButton::clicked, this, &JsonSenderGUI::onSend);
    connect(startReceiveBtn, &QPushButton::clicked, this, &JsonSenderGUI::onStartReceive);
    connect(stopReceiveBtn, &QPushButton::clicked, this, &JsonSenderGUI::onStopReceive);
    connect(&sender, &Sender::jsonReceived, this, &JsonSenderGUI::onJsonReceived);
}

void JsonSenderGUI::setupValidators()
{
    portValidator = new QIntValidator(1, 65535, this);
    portEdit->setValidator(portValidator);
    receivePortEdit->setValidator(portValidator);
}

void JsonSenderGUI::updateConnectionState(bool connected)
{
    isConnected = connected;
    sendBtn->setEnabled(connected);
    connectBtn->setText(connected ? "Disconnect" : "Connect");
    
    // Disable connection settings when connected
    protocolCombo->setEnabled(!connected);
    hostEdit->setEnabled(!connected);
    portEdit->setEnabled(!connected);
}

void JsonSenderGUI::updateReceiveState(bool receiving)
{
    isReceiving = receiving;
    startReceiveBtn->setEnabled(!receiving);
    stopReceiveBtn->setEnabled(receiving);
    receivePortEdit->setEnabled(!receiving);
}

void JsonSenderGUI::logMessage(const QString &message, const QString &prefix)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2%3").arg(timestamp).arg(prefix).arg(message);
    logEdit->append(logEntry);
}

bool JsonSenderGUI::validateInputs()
{
    if (hostEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Host cannot be empty");
        return false;
    }
    
    bool ok;
    int port = portEdit->text().toInt(&ok);
    if (!ok || port < 1 || port > 65535) {
        QMessageBox::warning(this, "Invalid Input", "Port must be between 1 and 65535");
        return false;
    }
    
    return true;
}

void JsonSenderGUI::onConnect() {
    if (isConnected) {
        // Disconnect
        sender.disconnect();
        updateConnectionState(false);
        logMessage("Disconnected", "🔌 ");
        return;
    }
    
    if (!validateInputs()) return;

    QString proto = protocolCombo->currentText().toLower();
    QString host = hostEdit->text().trimmed();
    bool ok;
    int port = portEdit->text().toInt(&ok);

    if (!ok) {
        QMessageBox::warning(this, "Error", "Invalid port number");
        return;
    }

    bool connected = false;
    if (proto == "tcp") {
        connected = sender.connectTcp(host, static_cast<quint16>(port));
    } else {
        connected = sender.connectUdp(host, static_cast<quint16>(port));
    }

    updateConnectionState(connected);
    if (connected) {
        logMessage(QString("Connected to %1:%2 via %3").arg(host).arg(port).arg(proto.toUpper()), "✅ ");
    } else {
        logMessage(QString("Connection failed to %1:%2 via %3").arg(host).arg(port).arg(proto.toUpper()), "❌ ");
    }
}

void JsonSenderGUI::onSend() {
    if (!isConnected) {
        QMessageBox::warning(this, "Error", "Not connected");
        return;
    }
    
    QString jsonText = jsonEdit->toPlainText().trimmed();
    if (jsonText.isEmpty()) {
        QMessageBox::warning(this, "Error", "JSON message cannot be empty");
        return;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        QString errorMsg = QString("Invalid JSON at offset %1: %2")
                          .arg(error.offset).arg(error.errorString());
        QMessageBox::warning(this, "JSON Error", errorMsg);
        logMessage(errorMsg, "❌ ");
        return;
    }

    if (sender.sendJson) {
        sender.sendJson(doc);
        logMessage("Sent: " + jsonText, "→ ");
    } else {
        logMessage("Send function not available", "❌ ");
    }
}

void JsonSenderGUI::onStartReceive() {
    QString proto = protocolCombo->currentText().toLower();
    bool ok;
    int port = receivePortEdit->text().toInt(&ok);
    
    if (!ok || port < 1 || port > 65535) {
        QMessageBox::warning(this, "Error", "Invalid receive port number");
        return;
    }

    bool started = false;
    if (proto == "tcp") {
        started = sender.startTcpReceiver(static_cast<quint16>(port));
    } else {
        started = sender.startUdpReceiver(static_cast<quint16>(port));
    }

    updateReceiveState(started);
    if (started) {
        logMessage(QString("Started receiving on port %1 via %2").arg(port).arg(proto.toUpper()), "✅ ");
    } else {
        logMessage("Failed to start receiver", "❌ ");
    }
}

void JsonSenderGUI::onStopReceive() {
    sender.stopReceiver();
    updateReceiveState(false);
    logMessage("Stopped receiving", "🛑 ");
}

void JsonSenderGUI::onJsonReceived(const QJsonDocument &doc, const QString &protocol, const QString &senderInfo) {
    QString jsonText = doc.toJson(QJsonDocument::Indented);
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString message = QString("[%1] ← %2 from %3:\n%4\n")
                     .arg(timestamp).arg(protocol).arg(senderInfo).arg(jsonText);
    receivedEdit->append(message);
    logMessage(QString("Received %1 message from %2").arg(protocol).arg(senderInfo), "📨 ");
}
