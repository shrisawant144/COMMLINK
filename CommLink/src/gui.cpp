#include "../include/gui.h"
#include "../include/sender.h"
#include "../include/receiver.h"
#include "../include/historytab.h"
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
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QApplication>

CommLinkGUI::CommLinkGUI() {
    setWindowTitle("CommLink - Network Communication Tool");
    resize(700, 700);
    
    // Initialize database with better error handling
    if (!historyManager.initializeDatabase()) {
        QMessageBox::critical(this, "Database Error", 
            "Failed to initialize message history database.\n"
            "History features will be disabled.\n"
            "Please check file permissions and disk space.");
        // Could disable history tab here if needed
    }
    
    setupUI();
    setupValidators();
    updateConnectionState(false);
    updateReceiveState(false);
}

void CommLinkGUI::setupUI()
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

    // File operation buttons for JSON
    auto *jsonBtnLayout = new QHBoxLayout();
    loadJsonBtn = new QPushButton("📁 Load JSON");
    saveJsonBtn = new QPushButton("💾 Save JSON");
    jsonBtnLayout->addWidget(loadJsonBtn);
    jsonBtnLayout->addWidget(saveJsonBtn);

    sendBtn = new QPushButton("📤 Send JSON");

    sendMsgLayout->addWidget(new QLabel("JSON Message:"));
    sendMsgLayout->addWidget(jsonEdit);
    sendMsgLayout->addLayout(jsonBtnLayout);
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

    // File operation buttons for received messages
    auto *receivedBtnLayout = new QHBoxLayout();
    exportMessagesBtn = new QPushButton("📤 Export Messages");
    clearMessagesBtn = new QPushButton("🗑️ Clear Messages");
    receivedBtnLayout->addWidget(exportMessagesBtn);
    receivedBtnLayout->addWidget(clearMessagesBtn);
    receivedLayout->addLayout(receivedBtnLayout);

    receiveLayout->addWidget(receiveConnGroup);
    receiveLayout->addWidget(receiveCtrlGroup);
    receiveLayout->addWidget(receivedGroup);

    // History Tab
    auto *historyTab = new HistoryTab(&historyManager);
    tabWidget->addTab(historyTab, "📚 History");

    // Logs Tab
    auto *logTab = new QWidget();
    tabWidget->addTab(logTab, "📋 Logs");

    auto *logLayout = new QVBoxLayout(logTab);

    logEdit = new QTextEdit();
    logEdit->setReadOnly(true);

    // File operation button for logs
    auto *logBtnLayout = new QHBoxLayout();
    exportLogsBtn = new QPushButton("📋 Export Logs");
    logBtnLayout->addWidget(exportLogsBtn);
    logBtnLayout->addStretch();

    logLayout->addWidget(logEdit);
    logLayout->addLayout(logBtnLayout);

    // Connect signals
    connect(connectBtn, &QPushButton::clicked, this, &CommLinkGUI::onConnect);
    connect(sendBtn, &QPushButton::clicked, this, &CommLinkGUI::onSend);
    connect(startReceiveBtn, &QPushButton::clicked, this, &CommLinkGUI::onStartReceive);
    connect(stopReceiveBtn, &QPushButton::clicked, this, &CommLinkGUI::onStopReceive);
    connect(&receiver, &Receiver::jsonReceived, this, &CommLinkGUI::onJsonReceived);
    connect(loadJsonBtn, &QPushButton::clicked, this, &CommLinkGUI::onLoadJson);
    connect(saveJsonBtn, &QPushButton::clicked, this, &CommLinkGUI::onSaveJson);
    connect(exportLogsBtn, &QPushButton::clicked, this, &CommLinkGUI::onExportLogs);
    connect(exportMessagesBtn, &QPushButton::clicked, this, &CommLinkGUI::onExportMessages);
    connect(clearMessagesBtn, &QPushButton::clicked, this, &CommLinkGUI::onClearMessages);

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

void CommLinkGUI::setupValidators()
{
    portValidator = new QIntValidator(1, 65535, this);
    portEdit->setValidator(portValidator);
    receivePortEdit->setValidator(portValidator);

    // Load settings
    QSettings settings("CommLink", "CommLinkApp");
    hostEdit->setText(settings.value("sendHost", "127.0.0.1").toString());
    portEdit->setText(settings.value("sendPort", "5000").toString());
    protocolCombo->setCurrentText(settings.value("sendProtocol", "TCP").toString());
    receiveHostEdit->setText(settings.value("receiveHost", "0.0.0.0").toString());
    receivePortEdit->setText(settings.value("receivePort", "5001").toString());
    receiveProtocolCombo->setCurrentText(settings.value("receiveProtocol", "TCP").toString());
}

void CommLinkGUI::updateConnectionState(bool connected)
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

void CommLinkGUI::updateReceiveState(bool receiving)
{
    isReceiving = receiving;
    startReceiveBtn->setEnabled(!receiving);
    stopReceiveBtn->setEnabled(receiving);
    receiveHostEdit->setEnabled(!receiving);
    receivePortEdit->setEnabled(!receiving);
    receiveProtocolCombo->setEnabled(!receiving);

    updateStatusBar();
}

void CommLinkGUI::logMessage(const QString &message, const QString &prefix)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2%3").arg(timestamp).arg(prefix).arg(message);
    logEdit->append(logEntry);
}

bool CommLinkGUI::validateInputs()
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

void CommLinkGUI::onConnect() {
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

void CommLinkGUI::onSend() {
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

        // Save to history
        QString host = hostEdit->text().trimmed();
        int port = portEdit->text().toInt();
        if (!historyManager.saveMessage("sent", protocolCombo->currentText(), host, port, doc)) {
            logMessage("Failed to save sent message to history", "⚠️ ");
        }
    } else {
        logMessage("Send function not available", "❌ ");
    }
}

void CommLinkGUI::onStartReceive() {
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

void CommLinkGUI::onStopReceive() {
    receiver.disconnect();
    updateReceiveState(false);
    logMessage("Stopped receiving", "🛑 ");
}

void CommLinkGUI::onJsonReceived(const QJsonDocument &doc, const QString &protocol, const QString &senderInfo) {
    QString jsonText = doc.toJson(QJsonDocument::Indented);
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString message = QString("[%1] ← %2 from %3:\n%4\n")
                     .arg(timestamp).arg(protocol).arg(senderInfo).arg(jsonText);
    receivedEdit->append(message);
    logMessage(QString("Received %1 message from %2").arg(protocol).arg(senderInfo), "📨 ");

    // Save received message to history
    QString host = senderInfo.split(':').first(); // Extract host from senderInfo
    int port = receivePortEdit->text().toInt();
    if (!historyManager.saveMessage("received", protocol, host, port, doc, senderInfo)) {
        logMessage("Failed to save received message to history", "⚠️ ");
    }

    // Save settings on successful receive
    saveSettings();
}

void CommLinkGUI::updateStatusBar() {
    QString status = QString("Send: %1 | Receive: %2")
                    .arg(isConnected ? "Connected" : "Disconnected")
                    .arg(isReceiving ? "Active" : "Inactive");
    statusBar->showMessage(status);
}

void CommLinkGUI::saveSettings() {
    QSettings settings("CommLink", "CommLinkApp");
    settings.setValue("sendHost", hostEdit->text());
    settings.setValue("sendPort", portEdit->text());
    settings.setValue("sendProtocol", protocolCombo->currentText());
    settings.setValue("receiveHost", receiveHostEdit->text());
    settings.setValue("receivePort", receivePortEdit->text());
    settings.setValue("receiveProtocol", receiveProtocolCombo->currentText());
}

void CommLinkGUI::onLoadJson() {
    QString filePath = QFileDialog::getOpenFileName(this, "Load JSON Message", FileManager::getDefaultSaveLocation(), "JSON Files (*.json);;All Files (*)");
    if (!filePath.isEmpty()) {
        QString content = FileManager::loadJsonFromFile(filePath);
        if (!content.isEmpty()) {
            jsonEdit->setPlainText(content);
            logMessage("Loaded JSON from " + filePath, "📁 ");
            QMessageBox::information(this, "Success", "JSON file loaded successfully");
        } else {
            QMessageBox::warning(this, "Error", "Failed to load JSON file or file contains invalid JSON");
        }
    }
}

void CommLinkGUI::onSaveJson() {
    QString content = jsonEdit->toPlainText();
    if (content.isEmpty()) {
        QMessageBox::warning(this, "Error", "No JSON content to save");
        return;
    }
    QString filePath = QFileDialog::getSaveFileName(this, "Save JSON Message", FileManager::getDefaultSaveLocation() + "/message.json", "JSON Files (*.json);;All Files (*)");
    if (!filePath.isEmpty()) {
        if (FileManager::saveJsonToFile(content, filePath)) {
            logMessage("Saved JSON to " + filePath, "💾 ");
            QMessageBox::information(this, "Success", "JSON file saved successfully");
        } else {
            QMessageBox::warning(this, "Error", "Failed to save JSON file");
        }
    }
}

void CommLinkGUI::onExportLogs() {
    QStringList logs = logEdit->toPlainText().split('\n');
    QString filePath = QFileDialog::getSaveFileName(this, "Export Logs", FileManager::getDefaultSaveLocation() + "/logs.txt", "Text Files (*.txt);;CSV Files (*.csv);;All Files (*)");
    if (!filePath.isEmpty()) {
        QString format = filePath.endsWith(".csv") ? "csv" : "txt";
        if (ExportManager::exportLogs(logs, format, filePath)) {
            logMessage("Exported logs to " + filePath, "📋 ");
            QMessageBox::information(this, "Success", "Logs exported successfully to: " + filePath);
        } else {
            QMessageBox::warning(this, "Error", "Failed to export logs");
        }
    }
}

void CommLinkGUI::onExportMessages() {
    // Parse received messages into QJsonDocument list
    QList<QJsonDocument> messages;
    QString receivedText = receivedEdit->toPlainText();
    QStringList lines = receivedText.split('\n');
    for (const QString& line : lines) {
        if (line.contains("{")) {
            int start = line.indexOf('{');
            int end = line.lastIndexOf('}');
            if (start != -1 && end != -1 && end > start) {
                QString jsonStr = line.mid(start, end - start + 1);
                QJsonParseError error;
                QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
                if (error.error == QJsonParseError::NoError) {
                    messages.append(doc);
                }
            }
        }
    }
    if (messages.isEmpty()) {
        QMessageBox::warning(this, "Error", "No valid messages to export");
        return;
    }
    QString filePath = QFileDialog::getSaveFileName(this, "Export Messages", FileManager::getDefaultSaveLocation() + "/messages.json", "JSON Files (*.json);;Text Files (*.txt);;CSV Files (*.csv);;All Files (*)");
    if (!filePath.isEmpty()) {
        QString format = "json";
        if (filePath.endsWith(".txt")) format = "txt";
        else if (filePath.endsWith(".csv")) format = "csv";
        
        if (ExportManager::exportMessages(messages, format, filePath)) {
            logMessage("Exported messages to " + filePath, "📤 ");
            QMessageBox::information(this, "Success", "Messages exported successfully to: " + filePath);
        } else {
            QMessageBox::warning(this, "Error", "Failed to export messages");
        }
    }
}

void CommLinkGUI::onClearMessages() {
    receivedEdit->clear();
    logMessage("Cleared received messages", "🗑️ ");
}

void CommLinkGUI::closeEvent(QCloseEvent *event) {
    // Disconnect all connections before closing
    if (isConnected) {
        sender.disconnect();
    }
    if (isReceiving) {
        receiver.disconnect();
    }
    
    // Save settings
    saveSettings();
    
    // Accept the close event
    event->accept();
    
    // Force application to quit
    QApplication::quit();
}
