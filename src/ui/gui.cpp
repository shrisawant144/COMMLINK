#include "commlink/ui/gui.h"
#include "commlink/ui/historytab.h"
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
#include <QtWidgets/QListWidget>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>
#include <QtCore/QUrl>
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
#include "commlink/ui/thememanager.h"

CommLinkGUI::CommLinkGUI() 
    : protocolCombo(nullptr), dataFormatCombo(nullptr), receiveProtocolCombo(nullptr),
      httpMethodCombo(nullptr), hostEdit(nullptr), portEdit(nullptr), receivePortEdit(nullptr),
      connectBtn(nullptr), sendBtn(nullptr), startReceiveBtn(nullptr), stopReceiveBtn(nullptr),
      loadJsonBtn(nullptr), saveJsonBtn(nullptr), exportLogsBtn(nullptr), exportMessagesBtn(nullptr),
      clearMessagesBtn(nullptr), jsonEdit(nullptr), logEdit(nullptr), receivedEdit(nullptr),
      statusBar(nullptr), menuBar(nullptr), lightModeAction(nullptr), darkModeAction(nullptr),
      autoModeAction(nullptr), logger(nullptr), clientStatusLabel(nullptr), serverStatusLabel(nullptr),
      sendModeCombo(nullptr), targetClientCombo(nullptr), connectedClientsList(nullptr),
      clientCountLabel(nullptr), portValidator(nullptr)
{
    // Note: Qt uses parent-child ownership for automatic memory management.
    // Raw pointers are correct here - Qt will delete child objects when parent is destroyed.
    // Linter warnings about "non-owner" pointers are false positives for Qt code.
    setWindowTitle("CommLink - Network Communication Tool");
    
    // Set window icon for title bar and taskbar
    QIcon appIcon;
    appIcon.addFile(":/assets/logo/CommLink_16.png", QSize(ICON_SIZE_16, ICON_SIZE_16));
    appIcon.addFile(":/assets/logo/CommLink_32.png", QSize(ICON_SIZE_32, ICON_SIZE_32));
    appIcon.addFile(":/assets/logo/CommLink_64.png", QSize(ICON_SIZE_64, ICON_SIZE_64));
    appIcon.addFile(":/assets/logo/CommLink_128.png", QSize(ICON_SIZE_128, ICON_SIZE_128));
    appIcon.addFile(":/assets/logo/CommLink_256.png", QSize(ICON_SIZE_256, ICON_SIZE_256));
    setWindowIcon(appIcon);
    
    resize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    setMinimumSize(MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
    
    // Initialize TCP client/server
    tcpClient = new TcpClient(this);
    connect(tcpClient, &TcpClient::connected, this, &CommLinkGUI::updateClientStatus);
    connect(tcpClient, &TcpClient::connected, this, [this]() {
        logMessage("TCP connection established", "[INFO] ");
    });
    connect(tcpClient, &TcpClient::disconnected, this, &CommLinkGUI::updateClientStatus);
    connect(tcpClient, &TcpClient::messageReceived, this, &CommLinkGUI::onDataReceived);
    connect(tcpClient, &TcpClient::errorOccurred, this, &CommLinkGUI::onWsError);
    
    tcpServer = new TcpServer(this);
    connect(tcpServer, &TcpServer::clientConnected, this, &CommLinkGUI::onClientConnected);
    connect(tcpServer, &TcpServer::clientDisconnected, this, &CommLinkGUI::onClientDisconnected);
    connect(tcpServer, &TcpServer::messageReceived, this, &CommLinkGUI::onDataReceived);
    connect(tcpServer, &TcpServer::errorOccurred, this, &CommLinkGUI::onWsError);
    
    // Initialize UDP client/server
    udpClient = new UdpClient(this);
    connect(udpClient, &UdpClient::connected, this, &CommLinkGUI::updateClientStatus);
    connect(udpClient, &UdpClient::disconnected, this, &CommLinkGUI::updateClientStatus);
    connect(udpClient, &UdpClient::messageReceived, this, &CommLinkGUI::onDataReceived);
    connect(udpClient, &UdpClient::errorOccurred, this, &CommLinkGUI::onWsError);
    
    udpServer = new UdpServer(this);
    connect(udpServer, &UdpServer::messageReceived, this, &CommLinkGUI::onDataReceived);
    connect(udpServer, &UdpServer::errorOccurred, this, &CommLinkGUI::onWsError);
    
    // Initialize WebSocket client/server
    wsClient = new WebSocketClient(this);
    connect(wsClient, &WebSocketClient::connected, this, &CommLinkGUI::updateClientStatus);
    connect(wsClient, &WebSocketClient::disconnected, this, &CommLinkGUI::updateClientStatus);
    connect(wsClient, &WebSocketClient::messageReceived, this, &CommLinkGUI::onDataReceived);
    connect(wsClient, &WebSocketClient::errorOccurred, this, &CommLinkGUI::onWsError);
    
    wsServer = new WebSocketServer(this);
    connect(wsServer, &WebSocketServer::clientConnected, this, &CommLinkGUI::onClientConnected);
    connect(wsServer, &WebSocketServer::clientDisconnected, this, &CommLinkGUI::onClientDisconnected);
    connect(wsServer, &WebSocketServer::messageReceived, this, &CommLinkGUI::onDataReceived);
    connect(wsServer, &WebSocketServer::errorOccurred, this, &CommLinkGUI::onWsError);
    
    // Initialize HTTP client
    httpClient = new HttpClient(this);
    connect(httpClient, &HttpClient::connected, this, &CommLinkGUI::updateClientStatus);
    connect(httpClient, &HttpClient::disconnected, this, &CommLinkGUI::updateClientStatus);
    connect(httpClient, &HttpClient::responseReceived, this, &CommLinkGUI::onDataReceived);
    connect(httpClient, &HttpClient::errorOccurred, this, &CommLinkGUI::onWsError);
    connect(httpClient, &HttpClient::requestSent, this, &CommLinkGUI::onHttpRequestSent);
    
    // Initialize HTTP server
    httpServer = new HttpServer(this);
    connect(httpServer, &HttpServer::clientConnected, this, &CommLinkGUI::onClientConnected);
    connect(httpServer, &HttpServer::clientDisconnected, this, &CommLinkGUI::onClientDisconnected);
    connect(httpServer, &HttpServer::messageReceived, this, &CommLinkGUI::onDataReceived);
    connect(httpServer, &HttpServer::errorOccurred, this, &CommLinkGUI::onWsError);
    
    // Initialize database with better error handling
    if (!historyManager.initializeDatabase()) {
        QMessageBox::critical(this, "Database Error", 
            "Failed to initialize message history database.\n"
            "History features will be disabled.\n"
            "Please check file permissions and disk space.");
    }
    
    setupUI();
    setupValidators();
    
    // Initialize theme
    ThemeManager::instance().loadSettings();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &CommLinkGUI::onThemeChanged);
    onThemeChanged();
}

void CommLinkGUI::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(MAIN_LAYOUT_SPACING);
    mainLayout->setContentsMargins(MAIN_LAYOUT_SPACING, MAIN_LAYOUT_SPACING, MAIN_LAYOUT_SPACING, MAIN_LAYOUT_SPACING);

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

    // Logo and Title Panel
    auto *logoPanel = new QWidget();
    auto *logoLayout = new QHBoxLayout(logoPanel);
    logoLayout->setContentsMargins(0, 0, 0, 0);

    auto *logoLabel = new QLabel();
    QPixmap logo(":/logo.svg");
    if (logo.isNull()) {
        // Fallback: create a simple colored placeholder if resource is missing
        logo = QPixmap(64, 64);
        logo.fill(QColor(0, 169, 157)); // CommLink teal color
    }
    logoLabel->setPixmap(logo.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setFixedSize(64, 64);

    auto *titleLabel = new QLabel("CommLink");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #007BFF;");
    titleLabel->setAlignment(Qt::AlignVCenter);

    logoLayout->addWidget(logoLabel);
    logoLayout->addWidget(titleLabel);
    logoLayout->addStretch();

    leftLayout->addWidget(logoPanel);

    // Connection Status Panel
    auto *statusPanel = new QGroupBox("Connection Status");
    auto *statusLayout = new QGridLayout(statusPanel);

    auto *clientLabel = new QLabel("Client:");
    clientStatusLabel = new QLabel("Disconnected");
    clientStatusLabel->setStyleSheet("color: #dc3545; font-weight: bold;");

    auto *serverLabel = new QLabel("Server:");
    serverStatusLabel = new QLabel("Stopped");
    serverStatusLabel->setStyleSheet("color: #dc3545; font-weight: bold;");

    statusLayout->addWidget(clientLabel, 0, 0);
    statusLayout->addWidget(clientStatusLabel, 0, 1);
    statusLayout->addWidget(serverLabel, 1, 0);
    statusLayout->addWidget(serverStatusLabel, 1, 1);

    leftLayout->addWidget(statusPanel);

    // Client Configuration
    auto *sendGroup = new QGroupBox("Client Configuration");
    auto *sendLayout = new QGridLayout(sendGroup);
    
    protocolCombo = new QComboBox();
    protocolCombo->addItems({"TCP", "UDP", "WebSocket", "HTTP"});
    protocolCombo->setMinimumHeight(32);
    connect(protocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &CommLinkGUI::onClientProtocolChanged);
    
    httpMethodCombo = new QComboBox();
    httpMethodCombo->addItems({"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"});
    httpMethodCombo->setCurrentText("POST");
    httpMethodCombo->setMinimumHeight(32);
    httpMethodCombo->setVisible(false);
    
    auto *clientInfoLabel = new QLabel("TCP/UDP: Host + Port | WebSocket: ws://host:port | HTTP: http://host:port/path");
    clientInfoLabel->setStyleSheet("color: #6c757d; font-size: 10px; font-style: italic;");
    clientInfoLabel->setWordWrap(true);
    
    hostEdit = new QLineEdit("127.0.0.1");
    hostEdit->setMinimumHeight(32);
    hostEdit->setPlaceholderText("Host/IP, ws://host:port, or http://host:port/path");
    
    portEdit = new QLineEdit("5000");
    portEdit->setMinimumHeight(32);
    portEdit->setPlaceholderText("Port number");
    portEdit->setObjectName("clientPortEdit");
    
    auto *hostLabel = new QLabel("Host:");
    auto *portLabel = new QLabel("Port:");
    portLabel->setObjectName("clientPortLabel");
    
    connectBtn = new QPushButton("Connect");
    connectBtn->setMinimumHeight(36);
    connectBtn->setStyleSheet("QPushButton { font-weight: bold; background-color: #007bff; color: white; }");
    
    sendLayout->addWidget(new QLabel("Protocol:"), 0, 0);
    sendLayout->addWidget(protocolCombo, 0, 1);
    sendLayout->addWidget(new QLabel("HTTP Method:"), 1, 0);
    sendLayout->addWidget(httpMethodCombo, 1, 1);
    sendLayout->addWidget(clientInfoLabel, 2, 0, 1, 2);
    sendLayout->addWidget(hostLabel, 3, 0);
    sendLayout->addWidget(hostEdit, 3, 1);
    sendLayout->addWidget(portLabel, 4, 0);
    sendLayout->addWidget(portEdit, 4, 1);
    sendLayout->addWidget(connectBtn, 5, 0, 1, 2);
    
    leftLayout->addWidget(sendGroup);

    // Server Configuration
    auto *receiveGroup = new QGroupBox("Server Configuration");
    auto *receiveLayout = new QGridLayout(receiveGroup);
    
    receiveProtocolCombo = new QComboBox();
    receiveProtocolCombo->addItems({"TCP", "UDP", "WebSocket", "HTTP"});
    receiveProtocolCombo->setMinimumHeight(32);
    connect(receiveProtocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &CommLinkGUI::onServerProtocolChanged);
    
    auto *serverInfoLabel = new QLabel("All protocols: Listen on port | TCP/WS/HTTP: Multiple clients | UDP: Connectionless");
    serverInfoLabel->setStyleSheet("color: #6c757d; font-size: 10px; font-style: italic;");
    serverInfoLabel->setWordWrap(true);
    
    receivePortEdit = new QLineEdit("5001");
    receivePortEdit->setMinimumHeight(32);
    receivePortEdit->setPlaceholderText("Listen port");
    
    auto *receiveButtonLayout = new QHBoxLayout();
    startReceiveBtn = new QPushButton("Start Server");
    startReceiveBtn->setMinimumHeight(36);
    startReceiveBtn->setStyleSheet("QPushButton { background-color: #28a745; color: white; font-weight: bold; }");
    
    stopReceiveBtn = new QPushButton("Stop Server");
    stopReceiveBtn->setMinimumHeight(36);
    stopReceiveBtn->setStyleSheet("QPushButton { background-color: #dc3545; color: white; font-weight: bold; }");
    stopReceiveBtn->setEnabled(false);
    
    receiveButtonLayout->addWidget(startReceiveBtn);
    receiveButtonLayout->addWidget(stopReceiveBtn);
    
    // Connected clients list
    clientCountLabel = new QLabel("Connected Clients: 0");
    clientCountLabel->setStyleSheet("font-weight: bold; color: #6c757d;");
    
    connectedClientsList = new QListWidget();
    connectedClientsList->setMaximumHeight(80);
    connectedClientsList->setStyleSheet("QListWidget { border: 1px solid #dee2e6; border-radius: 4px; }");
    
    receiveLayout->addWidget(new QLabel("Protocol:"), 0, 0);
    receiveLayout->addWidget(receiveProtocolCombo, 0, 1);
    receiveLayout->addWidget(new QLabel("Port:"), 1, 0);
    receiveLayout->addWidget(receivePortEdit, 1, 1);
    receiveLayout->addWidget(new QLabel("Controls:"), 2, 0);
    receiveLayout->addLayout(receiveButtonLayout, 2, 1);
    receiveLayout->addWidget(clientCountLabel, 3, 0, 1, 2);
    receiveLayout->addWidget(connectedClientsList, 4, 0, 1, 2);
    
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
    
    // Send Mode and Target Selection
    auto *sendControlGroup = new QGroupBox("Send Configuration");
    auto *sendControlLayout = new QGridLayout(sendControlGroup);
    
    sendModeCombo = new QComboBox();
    sendModeCombo->addItem("Send as Client", 0);
    sendModeCombo->addItem("Send as Server (Broadcast)", 1);
    sendModeCombo->addItem("Send as Server (To Selected)", 2);
    sendModeCombo->setMinimumHeight(32);
    connect(sendModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &CommLinkGUI::onSendModeChanged);
    
    targetClientCombo = new QComboBox();
    targetClientCombo->setMinimumHeight(32);
    targetClientCombo->setEnabled(false);
    
    sendControlLayout->addWidget(new QLabel("Mode:"), 0, 0);
    sendControlLayout->addWidget(sendModeCombo, 0, 1);
    sendControlLayout->addWidget(new QLabel("Target:"), 1, 0);
    sendControlLayout->addWidget(targetClientCombo, 1, 1);
    
    sendTabLayout->addWidget(sendControlGroup);
    
    // Format selector
    auto *formatGroup = new QGroupBox("Message Format");
    auto *formatLayout = new QHBoxLayout(formatGroup);
    
    dataFormatCombo = new QComboBox();
    dataFormatCombo->addItem("JSON", static_cast<int>(DataFormatType::JSON));
    dataFormatCombo->addItem("XML", static_cast<int>(DataFormatType::XML));
    dataFormatCombo->addItem("CSV", static_cast<int>(DataFormatType::CSV));
    dataFormatCombo->addItem("Text", static_cast<int>(DataFormatType::TEXT));
    dataFormatCombo->addItem("Binary", static_cast<int>(DataFormatType::BINARY));
    dataFormatCombo->addItem("Hex", static_cast<int>(DataFormatType::HEX));
    dataFormatCombo->setMinimumHeight(32);
    connect(dataFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &CommLinkGUI::onFormatChanged);
    
    formatLayout->addWidget(new QLabel("Format:"));
    formatLayout->addWidget(dataFormatCombo);
    formatLayout->addStretch();
    
    sendTabLayout->addWidget(formatGroup);
    
    // Message editor
    auto *messageGroup = new QGroupBox("Message Content");
    auto *messageLayout = new QVBoxLayout(messageGroup);
    
    auto *messageLabel = new QLabel("Message:");
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
    sendBtn->setEnabled(false);
    
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
    
    // Logs Tab with professional logger widget
    auto *logTab = new QWidget();
    auto *logTabLayout = new QVBoxLayout(logTab);
    logTabLayout->setContentsMargins(0, 0, 0, 0);
    
    logger = new LoggerWidget();
    logger->setMaxLines(1000);
    
    auto *logButtonLayout = new QHBoxLayout();
    exportLogsBtn = new QPushButton("Export Logs");
    exportLogsBtn->setMinimumHeight(32);
    
    auto *clearLogsBtn = new QPushButton("Clear Logs");
    clearLogsBtn->setMinimumHeight(32);
    
    logButtonLayout->addWidget(exportLogsBtn);
    logButtonLayout->addWidget(clearLogsBtn);
    logButtonLayout->addStretch();
    
    logTabLayout->addWidget(logger);
    logTabLayout->addLayout(logButtonLayout);
    
    tabWidget->addTab(logTab, "Logs");
    
    // Connect clear logs button
    connect(clearLogsBtn, &QPushButton::clicked, logger, &LoggerWidget::clear);
    
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
            messageLabel->setText("Binary Message:");
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
    connect(loadJsonBtn, &QPushButton::clicked, this, &CommLinkGUI::onLoadMessage);
    connect(saveJsonBtn, &QPushButton::clicked, this, &CommLinkGUI::onSaveMessage);
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
    portValidator = new QIntValidator(1, MAX_PORT_NUMBER, this);
    portEdit->setValidator(portValidator);
    receivePortEdit->setValidator(portValidator);

    // Load settings
    QSettings settings("CommLink", "CommLinkApp");
    hostEdit->setText(settings.value("sendHost", "127.0.0.1").toString());
    portEdit->setText(settings.value("sendPort", "5000").toString());
    protocolCombo->setCurrentText(settings.value("sendProtocol", "TCP").toString());
    receivePortEdit->setText(settings.value("receivePort", "5001").toString());
    receiveProtocolCombo->setCurrentText(settings.value("receiveProtocol", "TCP").toString());
}

void CommLinkGUI::logMessage(const QString &message, const QString &prefix)
{
    if (!logger) return;
    
    // Determine log level based on prefix
    if (prefix.contains("ERROR") || prefix.contains("FAIL")) {
        logger->logError(message);
    } else if (prefix.contains("WARN")) {
        logger->logWarning(message);
    } else if (prefix.contains("SUCCESS")) {
        logger->logSuccess(message);
    } else if (prefix.contains("DEBUG")) {
        logger->logDebug(message);
    } else {
        logger->logInfo(message);
    }
}

bool CommLinkGUI::validateInputs()
{
    if (hostEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Host cannot be empty");
        return false;
    }
    
    bool ok;
    int port = portEdit->text().toInt(&ok);
    if (!ok || port < 1 || port > MAX_PORT_NUMBER) {
        QMessageBox::warning(this, "Invalid Input", QString("Port must be between 1 and %1").arg(MAX_PORT_NUMBER));
        return false;
    }
    
    return true;
}

void CommLinkGUI::onConnect() {
    QString proto = protocolCombo->currentText();
    DataFormatType format = static_cast<DataFormatType>(dataFormatCombo->currentData().toInt());
    
    // Handle HTTP
    if (proto == "HTTP") {
        if (httpClient->isConnected()) {
            httpClient->disconnect();
            updateClientStatus();
            updateSendButtonState();
            return;
        }
        
        QString url = hostEdit->text().trimmed();
        if (url.isEmpty()) {
            QMessageBox::warning(this, "Invalid URL", "Please enter an HTTP URL");
            return;
        }
        if (!url.startsWith("http://") && !url.startsWith("https://")) {
            url = "http://" + url;
        }
        
        // Set HTTP client as "connected" (ready to send)
        httpClient->setFormat(format);
        // Manually trigger connected state for HTTP
        connect(httpClient, &HttpClient::connected, this, [this]() {
            updateClientStatus();
            updateSendButtonState();
        }, Qt::UniqueConnection);
        
        // Simulate connection by emitting the signal
        QMetaObject::invokeMethod(httpClient, [this]() {
            httpClient->setConnected(true);
        }, Qt::QueuedConnection);
        
        logMessage("HTTP client ready: " + url, "[HTTP] ");
        return;
    }
    
    // Handle WebSocket
    if (proto == "WebSocket") {
        if (wsClient->isConnected()) {
            wsClient->disconnect();
            updateClientStatus();
            updateSendButtonState();
            return;
        }
        
        QString url = hostEdit->text().trimmed();
        if (url.isEmpty()) {
            QMessageBox::warning(this, "Invalid URL", "Please enter a WebSocket URL");
            return;
        }
        if (!url.startsWith("ws://") && !url.startsWith("wss://")) {
            url = "ws://" + url;
        }
        wsClient->setFormat(format);
        wsClient->connectToServer(url);
        logMessage("Connecting to " + url + "...", "[WS] ");
        return;
    }
    
    // Handle TCP
    if (proto == "TCP") {
        if (tcpClient->isConnected()) {
            tcpClient->disconnect();
            updateClientStatus();
            updateSendButtonState();
            return;
        }
        
        if (!validateInputs()) return;
        QString host = hostEdit->text().trimmed();
        int port = portEdit->text().toInt();
        
        tcpClient->setFormat(format);
        tcpClient->connectToHost(host, static_cast<quint16>(port));
        logMessage(QString("Connecting to %1:%2 via TCP...").arg(host).arg(port), "[INFO] ");
        return;
    }
    
    // Handle UDP
    if (proto == "UDP") {
        if (udpClient->isConnected()) {
            udpClient->disconnect();
            updateClientStatus();
            updateSendButtonState();
            return;
        }
        
        if (!validateInputs()) return;
        QString host = hostEdit->text().trimmed();
        int port = portEdit->text().toInt();
        
        udpClient->setFormat(format);
        if (udpClient->connectToHost(host, static_cast<quint16>(port))) {
            logMessage(QString("Connected to %1:%2 via UDP").arg(host).arg(port), "[INFO] ");
        } else {
            logMessage("UDP connection failed", "[ERROR] ");
        }
        return;
    }
}

void CommLinkGUI::onSend() {
    QString proto = protocolCombo->currentText();
    
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

    QVariant parsedData = DataMessage::parseInput(messageText, format);
    DataMessage msg(format, parsedData);

    // Send via appropriate client
    if (proto == "HTTP") {
        QString url = hostEdit->text().trimmed();
        if (!url.startsWith("http://") && !url.startsWith("https://")) {
            url = "http://" + url;
        }
        
        QString methodStr = httpMethodCombo->currentText();
        HttpClient::Method method = HttpClient::POST;
        if (methodStr == "GET") method = HttpClient::GET;
        else if (methodStr == "POST") method = HttpClient::POST;
        else if (methodStr == "PUT") method = HttpClient::PUT;
        else if (methodStr == "DELETE") method = HttpClient::DELETE;
        else if (methodStr == "PATCH") method = HttpClient::PATCH;
        else if (methodStr == "HEAD") method = HttpClient::HEAD;
        else if (methodStr == "OPTIONS") method = HttpClient::OPTIONS;
        
        httpClient->sendRequest(url, method, msg);
        logMessage(QString("Sent via HTTP %1: %2").arg(methodStr).arg(messageText), "[HTTP-SEND] ");
        historyManager.saveMessage("sent", "HTTP", url, 0, msg);
    }
    else if (proto == "WebSocket" && wsClient->isConnected()) {
        wsClient->sendMessage(msg);
        logMessage("Sent via WebSocket: " + messageText, "[WS-SEND] ");
        historyManager.saveMessage("sent", "WebSocket", hostEdit->text(), 0, msg);
    }
    else if (proto == "TCP" && tcpClient->isConnected()) {
        tcpClient->sendMessage(msg);
        logMessage("Sent via TCP: " + messageText, "[TCP-SEND] ");
        historyManager.saveMessage("sent", "TCP", hostEdit->text(), portEdit->text().toInt(), msg);
    }
    else if (proto == "UDP" && udpClient->isConnected()) {
        udpClient->sendMessage(msg);
        logMessage("Sent via UDP: " + messageText, "[UDP-SEND] ");
        historyManager.saveMessage("sent", "UDP", hostEdit->text(), portEdit->text().toInt(), msg);
    }
    else {
        QMessageBox::warning(this, "Error", "Not connected");
    }
}

void CommLinkGUI::onStartReceive() {
    QString proto = receiveProtocolCombo->currentText();
    DataFormatType format = static_cast<DataFormatType>(dataFormatCombo->currentData().toInt());
    
    bool ok;
    int port = receivePortEdit->text().toInt(&ok);

    if (!ok || port < 1 || port > MAX_PORT_NUMBER) {
        QMessageBox::warning(this, "Error", QString("Invalid receive port number (must be 1-%1)").arg(MAX_PORT_NUMBER));
        return;
    }

    bool started = false;
    if (proto == "TCP") {
        tcpServer->setFormat(format);
        started = tcpServer->listen(static_cast<quint16>(port));
    } else if (proto == "UDP") {
        udpServer->setFormat(format);
        started = udpServer->listen(static_cast<quint16>(port));
    } else if (proto == "WebSocket") {
        wsServer->setFormat(format);
        started = wsServer->listen(static_cast<quint16>(port));
    } else if (proto == "HTTP") {
        httpServer->setFormat(format);
        started = httpServer->startServer(static_cast<quint16>(port));
    }

    if (started) {
        updateServerStatus();
        logMessage(QString("Started server on port %1 via %2").arg(port).arg(proto), "[INFO] ");
    } else {
        logMessage("Failed to start server", "[ERROR] ");
    }
}

void CommLinkGUI::onStopReceive() {
    tcpServer->close();
    udpServer->close();
    wsServer->close();
    httpServer->stopServer();
    connectedClientsList->clear();
    targetClientCombo->clear();
    updateServerStatus();
    logMessage("Stopped server", "[INFO] ");
}

void CommLinkGUI::onDataReceived(const DataMessage &msg, const QString &source, const QString &timestamp) {
    // Store the message for proper export
    receivedMessages.append(msg);

    // Determine protocol based on sender
    QString protocol = "Unknown";
    QObject *senderObj = sender();
    
    if (senderObj == tcpClient || senderObj == tcpServer) {
        protocol = "TCP";
    } else if (senderObj == udpClient || senderObj == udpServer) {
        protocol = "UDP";
    } else if (senderObj == wsClient || senderObj == wsServer) {
        protocol = "WebSocket";
    } else if (senderObj == httpClient || senderObj == httpServer) {
        protocol = "HTTP";
    }
    
    QString direction = "received";

    QString displayText = msg.toDisplayString();
    
    QString message = QString("[%1] ← %2 from %3:\n%4\n")
                     .arg(timestamp).arg(protocol).arg(source).arg(displayText);
    receivedEdit->append(message);
    logMessage(QString("Received %1 message from %2").arg(source).arg(timestamp), "[RECV] ");

    // Save received message to history
    QString host;
    int port = 0;
    
    // Parse host and port based on protocol and source format
    if (protocol == "HTTP") {
        // For HTTP, source can be a URL (http://host:port) or IP:port [method]
        if (source.startsWith("http://") || source.startsWith("https://")) {
            // Client side: source is URL like "http://127.0.0.1:5000"
            QUrl url(source.split(" ").first()); // Remove any trailing info like [HTTP 200]
            host = url.host();
            port = url.port(DEFAULT_HTTP_PORT);
        } else {
            // Server side: source is like "::ffff:127.0.0.1:42966 [POST /]"
            QString cleanSource = source.split(" ").first(); // Remove method info
            QStringList parts = cleanSource.split(":");
            if (parts.size() >= 2) {
                // Handle IPv6 format like ::ffff:127.0.0.1:42966
                port = parts.last().toInt();
                parts.removeLast();
                host = parts.join(":");
            } else {
                host = cleanSource;
                port = receivePortEdit->text().toInt();
            }
        }
    } else {
        // For TCP/UDP/WebSocket: source format is typically "host:port"
        QStringList parts = source.split(":");
        if (parts.size() >= 2) {
            port = parts.last().toInt();
            parts.removeLast();
            host = parts.join(":");
        } else {
            host = source;
            port = receivePortEdit->text().toInt();
        }
    }
    
    if (!historyManager.saveMessage(direction, protocol, host, port, msg, source)) {
        logMessage("Failed to save received message to history", "[WARN] ");
    }

    // Save settings on successful receive
    saveSettings();
}

void CommLinkGUI::updateStatusBar() {
    bool anyClientConnected = tcpClient->isConnected() || udpClient->isConnected() || 
                              wsClient->isConnected() || httpClient->isConnected();
    bool anyServerListening = tcpServer->isListening() || udpServer->isListening() || 
                              wsServer->isListening() || httpServer->isListening();
    
    QString sendStatus = anyClientConnected ? QString("TX: %1:%2").arg(hostEdit->text()).arg(portEdit->text()) : "TX: Idle";
    QString recvStatus = anyServerListening ? QString("RX: Port %1").arg(receivePortEdit->text()) : "RX: Idle";
    QString themeStatus = QString("UI: %1").arg(ThemeManager::instance().getThemeName());
    QString status = QString("%1 | %2 | %3").arg(sendStatus).arg(recvStatus).arg(themeStatus);
    statusBar->showMessage(status);
}

void CommLinkGUI::saveSettings() {
    QSettings settings("CommLink", "CommLinkApp");
    settings.setValue("sendHost", hostEdit->text());
    settings.setValue("sendPort", portEdit->text());
    settings.setValue("sendProtocol", protocolCombo->currentText());
    settings.setValue("receivePort", receivePortEdit->text());
    settings.setValue("receiveProtocol", receiveProtocolCombo->currentText());
}

void CommLinkGUI::onLoadMessage() {
    DataFormatType format = static_cast<DataFormatType>(dataFormatCombo->currentData().toInt());
    QString filter = QString("%1 Files (*.%2);;All Files (*)").arg(dataFormatCombo->currentText()).arg(FileManager::getFileExtension(format));
    QString filePath = QFileDialog::getOpenFileName(this, "Load Message", FileManager::getDefaultSaveLocation(), filter);
    if (!filePath.isEmpty()) {
        QString content = FileManager::loadMessageFromFile(filePath, format);
        if (!content.isEmpty()) {
            jsonEdit->setPlainText(content);
            logMessage("Loaded " + dataFormatCombo->currentText() + " from " + filePath, "[FILE] ");
            QMessageBox::information(this, "Success", dataFormatCombo->currentText() + " file loaded successfully");
        } else {
            QMessageBox::warning(this, "Error", "Failed to load file or file contains invalid " + dataFormatCombo->currentText());
        }
    }
}

void CommLinkGUI::onSaveMessage() {
    QString content = jsonEdit->toPlainText();
    if (content.isEmpty()) {
        QMessageBox::warning(this, "Error", "No content to save");
        return;
    }
    DataFormatType format = static_cast<DataFormatType>(dataFormatCombo->currentData().toInt());
    QString defaultName = QString("/message.%1").arg(FileManager::getFileExtension(format));
    QString filter = QString("%1 Files (*.%2);;All Files (*)").arg(dataFormatCombo->currentText()).arg(FileManager::getFileExtension(format));
    QString filePath = QFileDialog::getSaveFileName(this, "Save Message", FileManager::getDefaultSaveLocation() + defaultName, filter);
    if (!filePath.isEmpty()) {
        if (FileManager::saveMessageToFile(content, filePath, format)) {
            logMessage("Saved " + dataFormatCombo->currentText() + " to " + filePath, "[FILE] ");
            QMessageBox::information(this, "Success", dataFormatCombo->currentText() + " file saved successfully");
        } else {
            QMessageBox::warning(this, "Error", "Failed to save file");
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
    if (receivedMessages.isEmpty()) {
        QMessageBox::warning(this, "Error", "No messages to export");
        return;
    }
    QString filePath = QFileDialog::getSaveFileName(this, "Export Messages", FileManager::getDefaultSaveLocation() + "/messages.json", "JSON Files (*.json);;Text Files (*.txt);;CSV Files (*.csv);;All Files (*)");
    if (!filePath.isEmpty()) {
        QString format = "json";
        if (filePath.endsWith(".txt")) format = "txt";
        else if (filePath.endsWith(".csv")) format = "csv";

        if (ExportManager::exportMessages(receivedMessages, format, filePath)) {
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

void CommLinkGUI::onClientProtocolChanged(int index) {
    Q_UNUSED(index);
    QString proto = protocolCombo->currentText();
    
    // Disconnect all clients when switching protocols for smooth transition
    if (tcpClient->isConnected()) {
        tcpClient->disconnect();
    }
    if (udpClient->isConnected()) {
        udpClient->disconnect();
    }
    if (wsClient->isConnected()) {
        wsClient->disconnect();
    }
    if (httpClient->isConnected()) {
        httpClient->disconnect();
    }
    
    // Update UI status after disconnecting
    updateClientStatus();
    updateSendButtonState();
    
    // Find port field and label by object name
    auto *portLabel = findChild<QLabel*>("clientPortLabel");
    auto *portField = findChild<QLineEdit*>("clientPortEdit");
    
    if (proto == "HTTP") {
        // HTTP: URL only with method selector
        hostEdit->setPlaceholderText("http://host:port/path or https://host:port/path");
        if (portField) portField->setVisible(false);
        if (portLabel) portLabel->setVisible(false);
        if (httpMethodCombo) httpMethodCombo->setVisible(true);
    } else if (proto == "WebSocket") {
        // WebSocket: URL only (port is in URL)
        hostEdit->setPlaceholderText("ws://host:port or wss://host:port");
        if (portField) portField->setVisible(false);
        if (portLabel) portLabel->setVisible(false);
        if (httpMethodCombo) httpMethodCombo->setVisible(false);
    } else {
        // TCP/UDP: Separate host and port
        hostEdit->setPlaceholderText("Host/IP address");
        if (portField) portField->setVisible(true);
        if (portLabel) portLabel->setVisible(true);
        if (httpMethodCombo) httpMethodCombo->setVisible(false);
    }
    
    logMessage(QString("Switched to %1 protocol").arg(proto), "[INFO] ");
}

void CommLinkGUI::onServerProtocolChanged(int index) {
    Q_UNUSED(index);
    if (!connectedClientsList) return;
    
    QString proto = receiveProtocolCombo->currentText();
    
    // Stop all servers when switching protocols for smooth transition
    if (tcpServer->isListening()) {
        tcpServer->close();
    }
    if (udpServer->isListening()) {
        udpServer->close();
    }
    if (wsServer->isListening()) {
        wsServer->close();
    }
    if (httpServer->isListening()) {
        httpServer->stopServer();
    }
    
    // Clear client lists
    connectedClientsList->clear();
    targetClientCombo->clear();
    
    // Update UI status after stopping servers
    updateServerStatus();
    
    // TCP, WebSocket and HTTP support multiple clients, UDP doesn't
    bool showClientList = (proto == "TCP" || proto == "WebSocket" || proto == "HTTP");
    connectedClientsList->setVisible(showClientList);
    clientCountLabel->setVisible(showClientList);
    
    // Update label text based on protocol
    if (proto == "UDP") {
        clientCountLabel->setText("UDP: Connectionless (no client list)");
    } else {
        clientCountLabel->setText("Connected Clients: 0");
    }
    
    logMessage(QString("Switched server to %1 protocol").arg(proto), "[INFO] ");
}

void CommLinkGUI::onSendModeChanged(int index) {
    if (!targetClientCombo || !sendModeCombo) return;
    
    int mode = sendModeCombo->itemData(index).toInt();
    QString serverProto = receiveProtocolCombo->currentText();
    
    // Target selector only for TCP/WebSocket servers with "Send to Selected" mode
    bool enableTarget = (mode == 2) && (serverProto == "TCP" || serverProto == "WebSocket");
    targetClientCombo->setEnabled(enableTarget);
    
    // Update label based on mode
    if (mode == 1 && serverProto == "UDP") {
        // UDP broadcast is different
        logMessage("UDP server will reply to last sender", "[INFO] ");
    }
    
    updateSendButtonState();
}

void CommLinkGUI::onFormatChanged(int index) {
    Q_UNUSED(index);
    DataFormatType format = static_cast<DataFormatType>(dataFormatCombo->currentData().toInt());
    
    // Sync format to all active clients and servers
    tcpClient->setFormat(format);
    tcpServer->setFormat(format);
    udpClient->setFormat(format);
    udpServer->setFormat(format);
    wsClient->setFormat(format);
    wsServer->setFormat(format);
    
    logMessage(QString("Format changed to: %1").arg(dataFormatCombo->currentText()), "[INFO] ");
}

void CommLinkGUI::onClientConnected(const QString& clientInfo) {
    if (!connectedClientsList) return;
    
    connectedClientsList->addItem(clientInfo);
    targetClientCombo->addItem(clientInfo);
    clientCountLabel->setText(QString("Connected Clients: %1").arg(connectedClientsList->count()));
    updateSendButtonState();
    logMessage("Client connected: " + clientInfo, "[SERVER] ");
}

void CommLinkGUI::onClientDisconnected(const QString& clientInfo) {
    if (!connectedClientsList) return;
    
    // Remove from list
    auto items = connectedClientsList->findItems(clientInfo, Qt::MatchExactly);
    for (auto item : items) {
        delete connectedClientsList->takeItem(connectedClientsList->row(item));
    }
    
    // Remove from target combo
    int idx = targetClientCombo->findText(clientInfo);
    if (idx >= 0) targetClientCombo->removeItem(idx);
    
    clientCountLabel->setText(QString("Connected Clients: %1").arg(connectedClientsList->count()));
    updateSendButtonState();
    logMessage("Client disconnected: " + clientInfo, "[SERVER] ");
}

void CommLinkGUI::updateClientStatus() {
    bool anyConnected = tcpClient->isConnected() || udpClient->isConnected() || 
                        wsClient->isConnected() || httpClient->isConnected();
    
    if (anyConnected) {
        QString proto = protocolCombo->currentText();
        clientStatusLabel->setText("Connected (" + proto + ")");
        clientStatusLabel->setStyleSheet("color: #28a745; font-weight: bold;");
        connectBtn->setText(proto == "HTTP" ? "Ready" : "Disconnect");
        connectBtn->setStyleSheet("QPushButton { font-weight: bold; background-color: #dc3545; color: white; }");
    } else {
        clientStatusLabel->setText("Disconnected");
        clientStatusLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
        connectBtn->setText("Connect");
        connectBtn->setStyleSheet("QPushButton { font-weight: bold; background-color: #007bff; color: white; }");
    }
    
    updateSendButtonState();
}

void CommLinkGUI::updateServerStatus() {
    bool anyListening = tcpServer->isListening() || udpServer->isListening() || 
                        wsServer->isListening() || httpServer->isListening();
    
    if (anyListening) {
        QString proto = receiveProtocolCombo->currentText();
        serverStatusLabel->setText("Listening (" + proto + ")");
        serverStatusLabel->setStyleSheet("color: #28a745; font-weight: bold;");
        startReceiveBtn->setEnabled(false);
        stopReceiveBtn->setEnabled(true);
    } else {
        serverStatusLabel->setText("Stopped");
        serverStatusLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
        startReceiveBtn->setEnabled(true);
        stopReceiveBtn->setEnabled(false);
    }
    
    updateSendButtonState();
}

void CommLinkGUI::updateSendButtonState() {
    if (!sendBtn || !connectedClientsList) return;
    
    bool clientConnected = tcpClient->isConnected() || udpClient->isConnected() || 
                          wsClient->isConnected() || httpClient->isConnected();
    bool serverHasClients = connectedClientsList->count() > 0;
    bool serverListening = tcpServer->isListening() || udpServer->isListening() || 
                          wsServer->isListening() || httpServer->isListening();
    
    int sendMode = sendModeCombo->currentData().toInt();
    
    bool canSend = false;
    if (sendMode == 0) {
        // Send as client (HTTP is always ready if selected)
        if (protocolCombo->currentText() == "HTTP") {
            canSend = true; // HTTP doesn't need persistent connection
        } else {
            canSend = clientConnected;
        }
    } else if (sendMode == 1) {
        // Broadcast from server
        canSend = serverListening && serverHasClients;
    } else if (sendMode == 2) {
        // Send to selected client
        canSend = serverListening && targetClientCombo->currentIndex() >= 0;
    }
    
    sendBtn->setEnabled(canSend);
}

void CommLinkGUI::updateFieldVisibility() {
    if (!protocolCombo || !hostEdit || !portEdit) return;
    
    QString clientProto = protocolCombo->currentText();
    
    // Find port label by object name
    QLabel* portLabel = findChild<QLabel*>("clientPortLabel");
    
    // Client-side visibility
    if (clientProto == "WebSocket") {
        hostEdit->setPlaceholderText("ws://host:port or wss://host:port");
        portEdit->setVisible(false);
        if (portLabel) portLabel->setVisible(false);
    } else {
        hostEdit->setPlaceholderText("Host/IP address");
        portEdit->setVisible(true);
        if (portLabel) portLabel->setVisible(true);
    }
}

void CommLinkGUI::onWsConnected() {
    updateClientStatus();
    logMessage("WebSocket connected successfully", "[WS] ");
    
    // Sync format with current selection
    DataFormatType format = static_cast<DataFormatType>(dataFormatCombo->currentData().toInt());
    wsClient->setFormat(format);
}

void CommLinkGUI::onWsDisconnected() {
    updateClientStatus();
    logMessage("WebSocket disconnected", "[WS] ");
}

void CommLinkGUI::onWsError(const QString& error) {
    logMessage("Error: " + error, "[ERROR] ");
    QMessageBox::critical(this, "Error", error);
}

void CommLinkGUI::onHttpRequestSent(const QString& method, const QString& url) {
    logMessage(QString("HTTP %1 request sent to %2").arg(method).arg(url), "[HTTP] ");
}

void CommLinkGUI::closeEvent(QCloseEvent *event) {
    // Disconnect all clients
    tcpClient->disconnect();
    udpClient->disconnect();
    wsClient->disconnect();
    httpClient->disconnect();
    
    // Close all servers
    tcpServer->close();
    udpServer->close();
    wsServer->close();
    httpServer->stopServer();
    
    // Save settings
    saveSettings();
    ThemeManager::instance().saveSettings();
    
    // Accept the close event
    event->accept();
    
    // Force application to quit
    QApplication::quit();
}
