#include "../include/gui.h"
#include "../include/sender.h"
#include "../include/receiver.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QStatusBar>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>
#include <QtGui/QIntValidator>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDateTime>
#include <QtCore/QThread>
#include <QtCore/QSettings>
#include <QtGui/QFont>
#include <QtGui/QPalette>

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
    auto *mainLayout = new QVBoxLayout(this);

    // Create tab widget
    auto *tabWidget = new QTabWidget();
    mainLayout->addWidget(tabWidget);

    // Status bar
    statusBar = new QStatusBar();
    mainLayout->addWidget(statusBar);

    // Sending Tab
    auto *sendTab = new QWidget();
    tabWidget->addTab(sendTab, "📤 Sending");

    auto *sendLayout = new QVBoxLayout(sendTab);

    // Connection settings for sending
    auto *sendConnGroup = new QGroupBox("Connection Settings");
    auto *sendConnLayout = new QFormLayout(sendConnGroup);

    protocolCombo = new QComboBox();
    protocolCombo->addItems({"TCP", "UDP"});
    hostEdit = new QLineEdit("127.0.0.1");
    portEdit = new QLineEdit("5000");
    connectBtn = new QPushButton("🔗 Connect");

    sendConnLayout->addRow("Protocol:", protocolCombo);
    sendConnLayout->addRow("Host:", hostEdit);
    sendConnLayout->addRow("Port:", portEdit);
    sendConnLayout->addRow(connectBtn);

    // Send section
    auto *sendGroup = new QGroupBox("Send JSON");
    auto *sendMsgLayout = new QVBoxLayout(sendGroup);

    jsonEdit = new QTextEdit();
    jsonEdit->setPlainText(R"({"type":"hello","from":"gui","value":42})");
    jsonEdit->setMaximumHeight(120);

    sendBtn = new QPushButton("📤 Send JSON");

    sendMsgLayout->addWidget(new QLabel("JSON Message:"));
    sendMsgLayout->addWidget(jsonEdit);
    sendMsgLayout->addWidget(sendBtn);

    sendLayout->addWidget(sendConnGroup);
    sendLayout->addWidget(sendGroup);

    // Receiving Tab
    auto *receiveTab = new QWidget();
    tabWidget->addTab(receiveTab, "📥 Receiving");

    auto *receiveLayout = new QVBoxLayout(receiveTab);

    // Receive connection settings
    auto *receiveConnGroup = new QGroupBox("Connection Settings");
    auto *receiveConnLayout = new QFormLayout(receiveConnGroup);

    receiveProtocolCombo = new QComboBox();
    receiveProtocolCombo->addItems({"TCP", "UDP"});
    receiveHostEdit = new QLineEdit("0.0.0.0");
    receivePortEdit = new QLineEdit("5001");

    receiveConnLayout->addRow("Protocol:", receiveProtocolCombo);
    receiveConnLayout->addRow("Bind Host:", receiveHostEdit);
    receiveConnLayout->addRow("Port:", receivePortEdit);

    // Receive controls
    auto *receiveCtrlGroup = new QGroupBox("Controls");
    auto *receiveCtrlLayout = new QHBoxLayout(receiveCtrlGroup);

    startReceiveBtn = new QPushButton("▶️ Start Receiving");
    stopReceiveBtn = new QPushButton("⏹️ Stop Receiving");

    receiveCtrlLayout->addWidget(startReceiveBtn);
    receiveCtrlLayout->addWidget(stopReceiveBtn);

    // Received messages
    auto *receivedGroup = new QGroupBox("Received Messages");
    auto *receivedLayout = new QVBoxLayout(receivedGroup);

    receivedEdit = new QTextEdit();
    receivedEdit->setReadOnly(true);
    receivedEdit->setMaximumHeight(200);

    receivedLayout->addWidget(receivedEdit);

    receiveLayout->addWidget(receiveConnGroup);
    receiveLayout->addWidget(receiveCtrlGroup);
    receiveLayout->addWidget(receivedGroup);

    // Logs Tab
    auto *logTab = new QWidget();
    tabWidget->addTab(logTab, "📋 Logs");

    auto *logLayout = new QVBoxLayout(logTab);

    logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    logLayout->addWidget(logEdit);

    // Connect signals
    connect(connectBtn, &QPushButton::clicked, this, &JsonSenderGUI::onConnect);
    connect(sendBtn, &QPushButton::clicked, this, &JsonSenderGUI::onSend);
    connect(startReceiveBtn, &QPushButton::clicked, this, &JsonSenderGUI::onStartReceive);
    connect(stopReceiveBtn, &QPushButton::clicked, this, &JsonSenderGUI::onStopReceive);
    connect(&receiver, &Receiver::jsonReceived, this, &JsonSenderGUI::onJsonReceived);

    // Set modern style
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 1ex;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 10px 0 10px;
        }
        QPushButton {
            padding: 8px 16px;
            border-radius: 4px;
            background-color: #f0f0f0;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
        QPushButton:pressed {
            background-color: #d0d0d0;
        }
        QLineEdit, QTextEdit, QComboBox {
            padding: 4px;
            border: 1px solid #cccccc;
            border-radius: 3px;
        }
    )");

    updateStatusBar();
}

void JsonSenderGUI::setupValidators()
{
    portValidator = new QIntValidator(1, 65535, this);
    portEdit->setValidator(portValidator);
    receivePortEdit->setValidator(portValidator);

    // Load settings
    QSettings settings("JsonSender", "JsonSenderApp");
    hostEdit->setText(settings.value("sendHost", "127.0.0.1").toString());
    portEdit->setText(settings.value("sendPort", "5000").toString());
    protocolCombo->setCurrentText(settings.value("sendProtocol", "TCP").toString());
    receiveHostEdit->setText(settings.value("receiveHost", "0.0.0.0").toString());
    receivePortEdit->setText(settings.value("receivePort", "5001").toString());
    receiveProtocolCombo->setCurrentText(settings.value("receiveProtocol", "TCP").toString());
}

void JsonSenderGUI::updateConnectionState(bool connected)
{
    isConnected = connected;
    sendBtn->setEnabled(connected);
    connectBtn->setText(connected ? "🔌 Disconnect" : "🔗 Connect");

    // Disable connection settings when connected
    protocolCombo->setEnabled(!connected);
    hostEdit->setEnabled(!connected);
    portEdit->setEnabled(!connected);

    updateStatusBar();
}

void JsonSenderGUI::updateReceiveState(bool receiving)
{
    isReceiving = receiving;
    startReceiveBtn->setEnabled(!receiving);
    stopReceiveBtn->setEnabled(receiving);
    receiveHostEdit->setEnabled(!receiving);
    receivePortEdit->setEnabled(!receiving);
    receiveProtocolCombo->setEnabled(!receiving);

    updateStatusBar();
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
    QString proto = receiveProtocolCombo->currentText().toLower();
    bool ok;
    int port = receivePortEdit->text().toInt(&ok);

    if (!ok || port < 1 || port > 65535) {
        QMessageBox::warning(this, "Error", "Invalid receive port number");
        return;
    }

    bool started = false;
    if (proto == "tcp") {
        started = receiver.connectTcp(static_cast<quint16>(port));
    } else {
        started = receiver.connectUdp(static_cast<quint16>(port));
    }

    updateReceiveState(started);
    if (started) {
        logMessage(QString("Started receiving on port %1 via %2").arg(port).arg(proto.toUpper()), "✅ ");
    } else {
        logMessage("Failed to start receiver", "❌ ");
    }
}

void JsonSenderGUI::onStopReceive() {
    receiver.disconnect();
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

    // Save settings on successful receive
    saveSettings();
}

void JsonSenderGUI::updateStatusBar() {
    QString status = QString("Send: %1 | Receive: %2")
                    .arg(isConnected ? "Connected" : "Disconnected")
                    .arg(isReceiving ? "Active" : "Inactive");
    statusBar->showMessage(status);
}

void JsonSenderGUI::saveSettings() {
    QSettings settings("JsonSender", "JsonSenderApp");
    settings.setValue("sendHost", hostEdit->text());
    settings.setValue("sendPort", portEdit->text());
    settings.setValue("sendProtocol", protocolCombo->currentText());
    settings.setValue("receiveHost", receiveHostEdit->text());
    settings.setValue("receivePort", receivePortEdit->text());
    settings.setValue("receiveProtocol", receiveProtocolCombo->currentText());
}
