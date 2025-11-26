#include "../include/gui.h"
#include "../include/sender.h"
#include "../include/receiver.h"
#include "../include/historytab.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QSplitter>
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
#include <QtWidgets/QActionGroup>
#include <QtGui/QIcon>
#include "../include/thememanager.h"

CommLinkGUI::CommLinkGUI() {
    setWindowTitle("CommLink - Network Communication Tool");
    resize(1000, 700);
    setMinimumSize(800, 600);
    
    // Initialize database with better error handling
    if (!historyManager.initializeDatabase()) {
        QMessageBox::critical(this, "Database Error", 
            "Failed to initialize message history database.\n"
            "History features will be disabled.\n"
            "Please check file permissions and disk space.");
    }
    
    setupUI();
    setupValidators();
    updateConnectionState(false);
    updateReceiveState(false);
    
    // Initialize theme
    ThemeManager::instance().loadSettings();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &CommLinkGUI::onThemeChanged);
    onThemeChanged();
}

void CommLinkGUI::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // Create menu bar
    menuBar = new QMenuBar(this);
    mainLayout->setMenuBar(menuBar);
    
    // Create View menu with theme options
    auto *viewMenu = menuBar->addMenu("&View");
    auto *themeMenu = viewMenu->addMenu("&Appearance");
    
    auto *themeGroup = new QActionGroup(this);
    
    lightModeAction = new QAction("&Light Mode", this);
    lightModeAction->setCheckable(true);
    lightModeAction->setActionGroup(themeGroup);
    themeMenu->addAction(lightModeAction);
    
    darkModeAction = new QAction("&Dark Mode", this);
    darkModeAction->setCheckable(true);
    darkModeAction->setActionGroup(themeGroup);
    themeMenu->addAction(darkModeAction);
    
    autoModeAction = new QAction("&System Default", this);
    autoModeAction->setCheckable(true);
    autoModeAction->setActionGroup(themeGroup);
    themeMenu->addAction(autoModeAction);

    // Main content area with horizontal splitter
    auto *splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(splitter);

    // Left panel - Communication
    auto *leftPanel = new QWidget();
    leftPanel->setMinimumWidth(400);
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(12);

    // Connection Status Panel
    auto *statusPanel = new QGroupBox("Connection Status");
    auto *statusLayout = new QGridLayout(statusPanel);
    
    auto *sendStatusLabel = new QLabel("Send:");
    auto *sendStatusValue = new QLabel("Disconnected");
    sendStatusValue->setObjectName("sendStatus");
    sendStatusValue->setStyleSheet("color: red; font-weight: bold;");
    
    auto *receiveStatusLabel = new QLabel("Receive:");
    auto *receiveStatusValue = new QLabel("Stopped");
    receiveStatusValue->setObjectName("receiveStatus");
    receiveStatusValue->setStyleSheet("color: red; font-weight: bold;");
    
    statusLayout->addWidget(sendStatusLabel, 0, 0);
    statusLayout->addWidget(sendStatusValue, 0, 1);
    statusLayout->addWidget(receiveStatusLabel, 1, 0);
    statusLayout->addWidget(receiveStatusValue, 1, 1);
    
    leftLayout->addWidget(statusPanel);

    // Send Configuration
    auto *sendGroup = new QGroupBox("Send Configuration");
    auto *sendLayout = new QGridLayout(sendGroup);
    
    protocolCombo = new QComboBox();
    protocolCombo->addItems({"TCP", "UDP"});
    protocolCombo->setMinimumHeight(32);
    
    hostEdit = new QLineEdit("127.0.0.1");
    hostEdit->setMinimumHeight(32);
    hostEdit->setPlaceholderText("Enter host address");
    
    portEdit = new QLineEdit("5000");
    portEdit->setMinimumHeight(32);
    portEdit->setPlaceholderText("Port number");
    
    connectBtn = new QPushButton("Connect");
    connectBtn->setMinimumHeight(36);
    connectBtn->setStyleSheet("QPushButton { font-weight: bold; }");
    
    sendLayout->addWidget(new QLabel("Protocol:"), 0, 0);
    sendLayout->addWidget(protocolCombo, 0, 1);
    sendLayout->addWidget(new QLabel("Host:"), 1, 0);
    sendLayout->addWidget(hostEdit, 1, 1);
    sendLayout->addWidget(new QLabel("Port:"), 2, 0);
    sendLayout->addWidget(portEdit, 2, 1);
    sendLayout->addWidget(connectBtn, 3, 0, 1, 2);
    
    leftLayout->addWidget(sendGroup);

    // Receive Configuration
    auto *receiveGroup = new QGroupBox("Receive Configuration");
    auto *receiveLayout = new QGridLayout(receiveGroup);
    
    receiveProtocolCombo = new QComboBox();
    receiveProtocolCombo->addItems({"TCP", "UDP"});
    receiveProtocolCombo->setMinimumHeight(32);
    
    receiveHostEdit = new QLineEdit("0.0.0.0");
    receiveHostEdit->setMinimumHeight(32);
    receiveHostEdit->setPlaceholderText("Bind address");
    
    receivePortEdit = new QLineEdit("5001");
    receivePortEdit->setMinimumHeight(32);
    receivePortEdit->setPlaceholderText("Listen port");
    
    auto *receiveButtonLayout = new QHBoxLayout();
    startReceiveBtn = new QPushButton("Start");
    startReceiveBtn->setMinimumHeight(36);
    startReceiveBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    
    stopReceiveBtn = new QPushButton("Stop");
    stopReceiveBtn->setMinimumHeight(36);
    stopReceiveBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
    
    receiveButtonLayout->addWidget(startReceiveBtn);
    receiveButtonLayout->addWidget(stopReceiveBtn);
    
    receiveLayout->addWidget(new QLabel("Protocol:"), 0, 0);
    receiveLayout->addWidget(receiveProtocolCombo, 0, 1);
    receiveLayout->addWidget(new QLabel("Bind Host:"), 1, 0);
    receiveLayout->addWidget(receiveHostEdit, 1, 1);
    receiveLayout->addWidget(new QLabel("Port:"), 2, 0);
    receiveLayout->addWidget(receivePortEdit, 2, 1);
    receiveLayout->addWidget(new QLabel("Controls:"), 3, 0);
    receiveLayout->addLayout(receiveButtonLayout, 3, 1);
    
    leftLayout->addWidget(receiveGroup);
    leftLayout->addStretch();
    
    splitter->addWidget(leftPanel);

    // Right panel - Message handling with tabs
    auto *tabWidget = new QTabWidget();
    tabWidget->setMinimumWidth(500);
    
    // Send Message Tab
    auto *sendTab = new QWidget();
    auto *sendTabLayout = new QVBoxLayout(sendTab);
    sendTabLayout->setSpacing(12);
    
    // Format selector
    auto *formatGroup = new QGroupBox("Message Format");
    auto *formatLayout = new QHBoxLayout(formatGroup);
    
    dataFormatCombo = new QComboBox();
    dataFormatCombo->addItem("JSON", static_cast<int>(DataFormatType::JSON));
    dataFormatCombo->addItem("XML", static_cast<int>(DataFormatType::XML));
    dataFormatCombo->addItem("CSV", static_cast<int>(DataFormatType::CSV));
    dataFormatCombo->addItem("Text", static_cast<int>(DataFormatType::TEXT));
    dataFormatCombo->addItem("Binary (Hex)", static_cast<int>(DataFormatType::BINARY));
    dataFormatCombo->addItem("Hex", static_cast<int>(DataFormatType::HEX));
    dataFormatCombo->setMinimumHeight(32);
    
    formatLayout->addWidget(new QLabel("Format:"));
    formatLayout->addWidget(dataFormatCombo);
    formatLayout->addStretch();
    
    sendTabLayout->addWidget(formatGroup);
    
    // Message editor
    auto *messageGroup = new QGroupBox("Message Content");
    auto *messageLayout = new QVBoxLayout(messageGroup);
    
    auto *messageLabel = new QLabel("JSON Message:");
    messageLabel->setObjectName("messageLabel");
    
    jsonEdit = new QTextEdit();
    jsonEdit->setPlainText(R"({"type":"hello","from":"gui","value":42})");
    jsonEdit->setMinimumHeight(200);
    jsonEdit->setFont(QFont("Consolas, Monaco, monospace", 10));
    
    // File operations
    auto *fileButtonLayout = new QHBoxLayout();
    loadJsonBtn = new QPushButton("Load File");
    saveJsonBtn = new QPushButton("Save File");
    loadJsonBtn->setMinimumHeight(32);
    saveJsonBtn->setMinimumHeight(32);
    
    fileButtonLayout->addWidget(loadJsonBtn);
    fileButtonLayout->addWidget(saveJsonBtn);
    fileButtonLayout->addStretch();
    
    // Send button
    sendBtn = new QPushButton("Send Message");
    sendBtn->setMinimumHeight(40);
    sendBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; font-size: 14px; }");
    
    messageLayout->addWidget(messageLabel);
    messageLayout->addWidget(jsonEdit);
    messageLayout->addLayout(fileButtonLayout);
    messageLayout->addWidget(sendBtn);
    
    sendTabLayout->addWidget(messageGroup);
    
    tabWidget->addTab(sendTab, "Send Message");
    
    // Received Messages Tab
    auto *receiveTab = new QWidget();
    auto *receiveTabLayout = new QVBoxLayout(receiveTab);
    
    auto *receivedGroup = new QGroupBox("Received Messages");
    auto *receivedLayout = new QVBoxLayout(receivedGroup);
    
    receivedEdit = new QTextEdit();
    receivedEdit->setReadOnly(true);
    receivedEdit->setFont(QFont("Consolas, Monaco, monospace", 9));
    
    auto *receivedButtonLayout = new QHBoxLayout();
    exportMessagesBtn = new QPushButton("Export Messages");
    clearMessagesBtn = new QPushButton("Clear All");
    exportMessagesBtn->setMinimumHeight(32);
    clearMessagesBtn->setMinimumHeight(32);
    
    receivedButtonLayout->addWidget(exportMessagesBtn);
    receivedButtonLayout->addWidget(clearMessagesBtn);
    receivedButtonLayout->addStretch();
    
    receivedLayout->addWidget(receivedEdit);
    receivedLayout->addLayout(receivedButtonLayout);
    
    receiveTabLayout->addWidget(receivedGroup);
    
    tabWidget->addTab(receiveTab, "Received Messages");
    
    // History Tab
    auto *historyTab = new HistoryTab(&historyManager);
    tabWidget->addTab(historyTab, "History");
    
    // Logs Tab
    auto *logTab = new QWidget();
    auto *logTabLayout = new QVBoxLayout(logTab);
    
    auto *logGroup = new QGroupBox("Application Logs");
    auto *logLayout = new QVBoxLayout(logGroup);
    
    logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    logEdit->setFont(QFont("Consolas, Monaco, monospace", 9));
    
    auto *logButtonLayout = new QHBoxLayout();
    exportLogsBtn = new QPushButton("Export Logs");
    exportLogsBtn->setMinimumHeight(32);
    
    logButtonLayout->addWidget(exportLogsBtn);
    logButtonLayout->addStretch();
    
    logLayout->addWidget(logEdit);
    logLayout->addLayout(logButtonLayout);
    
    logTabLayout->addWidget(logGroup);
    
    tabWidget->addTab(logTab, "Logs");
    
    splitter->addWidget(tabWidget);
    splitter->setSizes({400, 600});

    // Status bar
    statusBar = new QStatusBar();
    statusBar->setStyleSheet("QStatusBar { border-top: 1px solid #ccc; }");
    mainLayout->addWidget(statusBar);

    // Format change handler
    connect(dataFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, messageLabel](int index) {
        DataFormatType format = static_cast<DataFormatType>(dataFormatCombo->itemData(index).toInt());
        switch (format) {
        case DataFormatType::JSON:
            messageLabel->setText("JSON Message:");
            jsonEdit->setPlainText(R"({"type":"hello","from":"gui","value":42})");
            break;
        case DataFormatType::XML:
            messageLabel->setText("XML Message:");
            jsonEdit->setPlainText("<message><type>hello</type><from>gui</from><value>42</value></message>");
            break;
        case DataFormatType::CSV:
            messageLabel->setText("CSV Message:");
            jsonEdit->setPlainText("type,from,value\nhello,gui,42");
            break;
        case DataFormatType::TEXT:
            messageLabel->setText("Text Message:");
            jsonEdit->setPlainText("Hello from GUI");
            break;
        case DataFormatType::BINARY:
            messageLabel->setText("Binary Message (as Hex):");
            jsonEdit->setPlainText("48656c6c6f");
            break;
        case DataFormatType::HEX:
            messageLabel->setText("Hex Message:");
            jsonEdit->setPlainText("48 65 6c 6c 6f");
            break;
        }
    });

    // Connect all signals
    connect(connectBtn, &QPushButton::clicked, this, &CommLinkGUI::onConnect);
    connect(sendBtn, &QPushButton::clicked, this, &CommLinkGUI::onSend);
    connect(startReceiveBtn, &QPushButton::clicked, this, &CommLinkGUI::onStartReceive);
    connect(stopReceiveBtn, &QPushButton::clicked, this, &CommLinkGUI::onStopReceive);
    connect(&receiver, &Receiver::dataReceived, this, &CommLinkGUI::onDataReceived);
    connect(loadJsonBtn, &QPushButton::clicked, this, &CommLinkGUI::onLoadJson);
    connect(saveJsonBtn, &QPushButton::clicked, this, &CommLinkGUI::onSaveJson);
    connect(exportLogsBtn, &QPushButton::clicked, this, &CommLinkGUI::onExportLogs);
    connect(exportMessagesBtn, &QPushButton::clicked, this, &CommLinkGUI::onExportMessages);
    connect(clearMessagesBtn, &QPushButton::clicked, this, &CommLinkGUI::onClearMessages);
    
    // Connect theme actions
    connect(lightModeAction, &QAction::triggered, this, &CommLinkGUI::onToggleLightMode);
    connect(darkModeAction, &QAction::triggered, this, &CommLinkGUI::onToggleDarkMode);
    connect(autoModeAction, &QAction::triggered, this, &CommLinkGUI::onToggleAutoMode);

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
    connectBtn->setText(connected ? "Disconnect" : "Connect");
    
    // Update visual status
    auto *sendStatus = findChild<QLabel*>("sendStatus");
    if (sendStatus) {
        if (connected) {
            sendStatus->setText(QString("Connected (%1:%2)").arg(hostEdit->text()).arg(portEdit->text()));
            sendStatus->setStyleSheet("color: green; font-weight: bold;");
        } else {
            sendStatus->setText("Disconnected");
            sendStatus->setStyleSheet("color: red; font-weight: bold;");
        }
    }

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
    
    // Update visual status
    auto *receiveStatus = findChild<QLabel*>("receiveStatus");
    if (receiveStatus) {
        if (receiving) {
            receiveStatus->setText(QString("Listening on port %1").arg(receivePortEdit->text()));
            receiveStatus->setStyleSheet("color: green; font-weight: bold;");
        } else {
            receiveStatus->setText("Stopped");
            receiveStatus->setStyleSheet("color: red; font-weight: bold;");
        }
    }

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
        logMessage("Disconnected", "[INFO] ");
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
        logMessage(QString("Connected to %1:%2 via %3").arg(host).arg(port).arg(proto.toUpper()), "[INFO] ");
    } else {
        logMessage(QString("Connection failed to %1:%2 via %3").arg(host).arg(port).arg(proto.toUpper()), "[ERROR] ");
    }
}

void CommLinkGUI::onSend() {
    if (!isConnected) {
        QMessageBox::warning(this, "Error", "Not connected");
        return;
    }

    QString messageText = jsonEdit->toPlainText().trimmed();
    if (messageText.isEmpty()) {
        QMessageBox::warning(this, "Error", "Message cannot be empty");
        return;
    }

    DataFormatType format = static_cast<DataFormatType>(dataFormatCombo->currentData().toInt());
    if (!DataMessage::validateInput(messageText, format)) {
        QMessageBox::warning(this, "Input Error", "Invalid input for selected format");
        return;
    }

    QVariant data = DataMessage::parseInput(messageText, format);
    DataMessage msg(format, data);

    if (sender.sendData) {
        sender.sendData(msg);
        logMessage("Sent: " + messageText, "[SEND] ");

        // Save to history (only for JSON messages)
        if (msg.type == DataFormatType::JSON && msg.data.canConvert<QJsonDocument>()) {
            QString host = hostEdit->text().trimmed();
            int port = portEdit->text().toInt();
            QJsonDocument doc = msg.data.value<QJsonDocument>();
            if (!historyManager.saveMessage("sent", protocolCombo->currentText(), host, port, doc)) {
                logMessage("Failed to save sent message to history", "[WARN] ");
            }
        }
    } else {
        logMessage("Send function not available", "[ERROR] ");
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
        logMessage(QString("Started receiving on port %1 via %2").arg(port).arg(proto.toUpper()), "[INFO] ");
    } else {
        logMessage("Failed to start receiver", "[ERROR] ");
    }
}

void CommLinkGUI::onStopReceive() {
    receiver.disconnect();
    updateReceiveState(false);
    logMessage("Stopped receiving", "[INFO] ");
}

void CommLinkGUI::onDataReceived(const DataMessage &msg, const QString &protocol, const QString &senderInfo) {
    QString displayText = msg.toDisplayString();
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString message = QString("[%1] ← %2 from %3:\n%4\n")
                     .arg(timestamp).arg(protocol).arg(senderInfo).arg(displayText);
    receivedEdit->append(message);
    logMessage(QString("Received %1 message from %2").arg(protocol).arg(senderInfo), "[RECV] ");

    // Save received message to history (only for JSON messages)
    if (msg.type == DataFormatType::JSON && msg.data.canConvert<QJsonDocument>()) {
        QString host = senderInfo.split(':').first(); // Extract host from senderInfo
        int port = receivePortEdit->text().toInt();
        QJsonDocument doc = msg.data.value<QJsonDocument>();
        if (!historyManager.saveMessage("received", protocol, host, port, doc, senderInfo)) {
            logMessage("Failed to save received message to history", "[WARN] ");
        }
    }

    // Save settings on successful receive
    saveSettings();
}

void CommLinkGUI::updateStatusBar() {
    QString sendStatus = isConnected ? QString("TX: %1:%2").arg(hostEdit->text()).arg(portEdit->text()) : "TX: Idle";
    QString recvStatus = isReceiving ? QString("RX: Port %1").arg(receivePortEdit->text()) : "RX: Idle";
    QString themeStatus = QString("UI: %1").arg(ThemeManager::instance().getThemeName());
    QString status = QString("%1 | %2 | %3").arg(sendStatus).arg(recvStatus).arg(themeStatus);
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
            logMessage("Loaded JSON from " + filePath, "[FILE] ");
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
            logMessage("Saved JSON to " + filePath, "[FILE] ");
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
            logMessage("Exported logs to " + filePath, "[EXPORT] ");
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
            logMessage("Exported messages to " + filePath, "[EXPORT] ");
            QMessageBox::information(this, "Success", "Messages exported successfully to: " + filePath);
        } else {
            QMessageBox::warning(this, "Error", "Failed to export messages");
        }
    }
}

void CommLinkGUI::onClearMessages() {
    receivedEdit->clear();
    logMessage("Cleared received messages", "[INFO] ");
}

void CommLinkGUI::onThemeChanged() {
    ThemeManager::instance().applyTheme(this);
    
    // Update menu checkmarks
    auto currentTheme = ThemeManager::instance().currentTheme();
    lightModeAction->setChecked(currentTheme == ThemeManager::Light);
    darkModeAction->setChecked(currentTheme == ThemeManager::Dark);
    autoModeAction->setChecked(currentTheme == ThemeManager::Auto);
    
    // Update status bar to show current theme
    updateStatusBar();
}

void CommLinkGUI::onToggleLightMode() {
    ThemeManager::instance().setTheme(ThemeManager::Light);
    logMessage("Switched to Light theme", "[THEME] ");
}

void CommLinkGUI::onToggleDarkMode() {
    ThemeManager::instance().setTheme(ThemeManager::Dark);
    logMessage("Switched to Dark theme", "[THEME] ");
}

void CommLinkGUI::onToggleAutoMode() {
    ThemeManager::instance().setTheme(ThemeManager::Auto);
    logMessage("Switched to Auto theme (follows system)", "[THEME] ");
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
    ThemeManager::instance().saveSettings();
    
    // Accept the close event
    event->accept();
    
    // Force application to quit
    QApplication::quit();
}
