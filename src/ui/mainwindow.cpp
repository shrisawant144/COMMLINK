#include "commlink/ui/mainwindow.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QAbstractItemView>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QtWebSockets/QWebSocket>
#include <QtGui/QIcon>
#include <QtGui/QKeySequence>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include "commlink/core/filemanager.h"
#include "commlink/core/exportmanager.h"

/**
 * @brief MainWindow constructor - Initializes the application
 * 
 * This is the main initialization sequence:
 * 
 * 1. Set window properties (title, icon, size)
 * 2. Initialize database (MessageHistoryManager)
 * 3. initializeNetworkComponents() - Creates all 8 network objects and wires their signals
 * 4. setupUI() - Creates all 5 UI panels and arranges layout
 * 5. setupMenuBar() - Creates menu bar with theme options
 * 6. setupShortcuts() - Configures keyboard shortcuts
 * 7. setupConnections() - Wires UI panel signals to MainWindow slots
 * 8. loadSettings() - Restores window geometry and preferences
 * 9. Initialize theme manager and apply theme
 * 
 * @note Network component signals are connected in initializeNetworkComponents()
 * @note UI panel signals are connected in setupConnections()
 * @note Order matters: network components must be created before UI panels
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , connectionPanel(nullptr)
    , serverPanel(nullptr)
    , messagePanel(nullptr)
    , displayPanel(nullptr)
    , statusPanel(nullptr)
    , menuBar(nullptr)
    , lightModeAction(nullptr)
    , darkModeAction(nullptr)
    , autoModeAction(nullptr)
    , tcpClient(nullptr)
    , tcpServer(nullptr)
    , udpClient(nullptr)
    , udpServer(nullptr)
    , wsClient(nullptr)
    , wsServer(nullptr)
    , httpClient(nullptr)
    , httpServer(nullptr)
{
    setWindowTitle("CommLink - Network Communication Tool (Modular)");
    
    // Set window icon
    QIcon appIcon;
    appIcon.addFile(":/assets/logo/CommLink_16.png", QSize(16, 16));
    appIcon.addFile(":/assets/logo/CommLink_32.png", QSize(32, 32));
    appIcon.addFile(":/assets/logo/CommLink_64.png", QSize(64, 64));
    appIcon.addFile(":/assets/logo/CommLink_128.png", QSize(128, 128));
    appIcon.addFile(":/assets/logo/CommLink_256.png", QSize(256, 256));
    setWindowIcon(appIcon);
    
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    setMinimumSize(MIN_WIDTH, MIN_HEIGHT);
    
    // Initialize database
    if (!historyManager.initializeDatabase()) {
        QMessageBox::critical(this, "Database Error",
            "Failed to initialize message history database.\n"
            "History features will be disabled.\n"
            "Please check file permissions and disk space.");
    }
    
    initializeNetworkComponents();
    setupUI();
    setupMenuBar();
    setupShortcuts();
    setupConnections();
    loadSettings();
    
    // Initialize theme
    ThemeManager::instance().loadSettings();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, 
            this, &MainWindow::onThemeChanged);
    onThemeChanged();
    
    logMessage("CommLink started successfully", "[INFO] ");
}

/**
 * @brief Initializes all network components and wires their signals
 * 
 * Flow:
 * 1. Creates all 8 network objects (4 clients + 4 servers)
 * 2. Connects network component signals to MainWindow slots:
 *    - connected() → updateStatus()
 *    - disconnected() → updateStatus()
 *    - messageReceived() → onDataReceived()
 *    - errorOccurred() → onNetworkError()
 *    - clientConnected() → onClientConnected() (servers only)
 *    - clientDisconnected() → onClientDisconnected() (servers only)
 * 
 * @note All network components are created with MainWindow as parent
 * @note Parent-child relationship ensures automatic cleanup
 * @note Signals are connected immediately, ready for use
 */
void MainWindow::initializeNetworkComponents()
{
    // TCP
    tcpClient = new TcpClient(this);
    tcpServer = new TcpServer(this);
    
    // UDP
    udpClient = new UdpClient(this);
    udpServer = new UdpServer(this);
    
    // WebSocket
    wsClient = new WebSocketClient(this);
    wsServer = new WebSocketServer(this);
    
    // HTTP
    httpClient = new HttpClient(this);
    httpServer = new HttpServer(this);
    
    // Connect TCP client signals
    connect(tcpClient, &TcpClient::connected, this, &MainWindow::updateStatus);
    connect(tcpClient, &TcpClient::disconnected, this, &MainWindow::updateStatus);
    connect(tcpClient, &TcpClient::messageReceived, this, &MainWindow::onDataReceived);
    connect(tcpClient, &TcpClient::errorOccurred, this, &MainWindow::onNetworkError);
    
    // Connect UDP client signals
    connect(udpClient, &UdpClient::connected, this, &MainWindow::updateStatus);
    connect(udpClient, &UdpClient::disconnected, this, &MainWindow::updateStatus);
    connect(udpClient, &UdpClient::messageReceived, this, &MainWindow::onDataReceived);
    connect(udpClient, &UdpClient::errorOccurred, this, &MainWindow::onNetworkError);
    
    // Connect WebSocket client signals
    connect(wsClient, &WebSocketClient::connected, this, &MainWindow::updateStatus);
    connect(wsClient, &WebSocketClient::disconnected, this, &MainWindow::updateStatus);
    connect(wsClient, &WebSocketClient::messageReceived, this, &MainWindow::onDataReceived);
    connect(wsClient, &WebSocketClient::errorOccurred, this, &MainWindow::onNetworkError);
    
    // Connect HTTP client signals
    connect(httpClient, &HttpClient::connected, this, &MainWindow::updateStatus);
    connect(httpClient, &HttpClient::disconnected, this, &MainWindow::updateStatus);
    connect(httpClient, &HttpClient::responseReceived, this, &MainWindow::onDataReceived);
    connect(httpClient, &HttpClient::errorOccurred, this, &MainWindow::onNetworkError);
    
    // Connect TCP server signals
    connect(tcpServer, &TcpServer::clientConnected, this, &MainWindow::onClientConnected);
    connect(tcpServer, &TcpServer::clientDisconnected, this, &MainWindow::onClientDisconnected);
    connect(tcpServer, &TcpServer::messageReceived, this, &MainWindow::onDataReceived);
    connect(tcpServer, &TcpServer::errorOccurred, this, &MainWindow::onNetworkError);
    
    // Connect UDP server signals
    connect(udpServer, &UdpServer::messageReceived, this, &MainWindow::onDataReceived);
    connect(udpServer, &UdpServer::errorOccurred, this, &MainWindow::onNetworkError);
    
    // Connect WebSocket server signals
    connect(wsServer, &WebSocketServer::clientConnected, this, &MainWindow::onClientConnected);
    connect(wsServer, &WebSocketServer::clientDisconnected, this, &MainWindow::onClientDisconnected);
    connect(wsServer, &WebSocketServer::messageReceived, this, &MainWindow::onDataReceived);
    connect(wsServer, &WebSocketServer::errorOccurred, this, &MainWindow::onNetworkError);
    
    // Connect HTTP server signals
    connect(httpServer, &HttpServer::clientConnected, this, &MainWindow::onClientConnected);
    connect(httpServer, &HttpServer::clientDisconnected, this, &MainWindow::onClientDisconnected);
    connect(httpServer, &HttpServer::messageReceived, this, &MainWindow::onDataReceived);
    connect(httpServer, &HttpServer::errorOccurred, this, &MainWindow::onNetworkError);
    
    // HTTP-specific signals
    connect(httpClient, &HttpClient::pollingStopped, this, [this](const QString& reason) {
        logMessage(QString("HTTP polling stopped: %1").arg(reason), "[WARN] ");
        QMessageBox::warning(this, "Polling Stopped",
            QString("Long-polling has been disabled:\n%1\n\nPlease check server availability.").arg(reason));
    });
}

void MainWindow::setupUI()
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    
    // Main splitter for left and right panels
    auto *mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setChildrenCollapsible(false);
    
    // Left panel - Controls
    auto *leftWidget = new QWidget();
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setSpacing(12);
    
    connectionPanel = new ConnectionPanel();
    serverPanel = new ServerPanel();
    messagePanel = new MessagePanel();
    
    leftLayout->addWidget(connectionPanel);
    leftLayout->addWidget(serverPanel);
    leftLayout->addWidget(messagePanel);
    leftLayout->addStretch();
    
    // Right panel - Display and Status
    auto *rightWidget = new QWidget();
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setSpacing(12);
    
    displayPanel = new DisplayPanel(&historyManager);
    statusPanel = new StatusPanel();
    
    rightLayout->addWidget(displayPanel, 1);
    rightLayout->addWidget(statusPanel);
    
    // Add to splitter
    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 2);
    
    mainLayout->addWidget(mainSplitter);
}

void MainWindow::setupMenuBar()
{
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // View menu
    auto *viewMenu = menuBar->addMenu("&View");
    auto *themeMenu = viewMenu->addMenu("&Appearance");
    
    auto *themeGroup = new QActionGroup(this);
    
    lightModeAction = new QAction("&Light Mode", this);
    lightModeAction->setCheckable(true);
    lightModeAction->setActionGroup(themeGroup);
    connect(lightModeAction, &QAction::triggered, this, &MainWindow::onToggleLightMode);
    themeMenu->addAction(lightModeAction);
    
    darkModeAction = new QAction("&Dark Mode", this);
    darkModeAction->setCheckable(true);
    darkModeAction->setActionGroup(themeGroup);
    connect(darkModeAction, &QAction::triggered, this, &MainWindow::onToggleDarkMode);
    themeMenu->addAction(darkModeAction);
    
    autoModeAction = new QAction("&System Default", this);
    autoModeAction->setCheckable(true);
    autoModeAction->setActionGroup(themeGroup);
    autoModeAction->setChecked(true);
    connect(autoModeAction, &QAction::triggered, this, &MainWindow::onToggleAutoMode);
    themeMenu->addAction(autoModeAction);
    
    // Help menu
    auto *helpMenu = menuBar->addMenu("&Help");
    auto *shortcutsAction = new QAction("Keyboard &Shortcuts", this);
    shortcutsAction->setShortcut(QKeySequence("F1"));
    connect(shortcutsAction, &QAction::triggered, this, &MainWindow::showShortcutsHelp);
    helpMenu->addAction(shortcutsAction);
}

void MainWindow::setupShortcuts()
{
    // Send message (Ctrl+Return)
    auto *sendShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return), this);
    connect(sendShortcut, &QShortcut::activated, this, &MainWindow::onSendRequested);
    
    // Connect/Disconnect (Ctrl+K)
    auto *connectShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_K), this);
    connect(connectShortcut, &QShortcut::activated, this, &MainWindow::onConnectRequested);
    
    // Load file (Ctrl+O)
    auto *loadShortcut = new QShortcut(QKeySequence::Open, this);
    connect(loadShortcut, &QShortcut::activated, this, &MainWindow::onLoadMessageRequested);
    
    // Save file (Ctrl+S)
    auto *saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::onSaveMessageRequested);
    
    // Clear messages (Ctrl+L)
    auto *clearShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this);
    connect(clearShortcut, &QShortcut::activated, displayPanel, &DisplayPanel::clearAllMessages);
    
    // Export messages (Ctrl+E)
    auto *exportShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), this);
    connect(exportShortcut, &QShortcut::activated, this, &MainWindow::onExportMessagesRequested);
    
    // Export logs (Ctrl+Shift+E)
    auto *exportLogsShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_E), this);
    connect(exportLogsShortcut, &QShortcut::activated, this, &MainWindow::onExportLogsRequested);
    
    // Start/Stop Server (Ctrl+R)
    auto *serverShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), this);
    connect(serverShortcut, &QShortcut::activated, this, [this]() {
        if (serverPanel->isServerRunning()) {
            onStopServerRequested();
        } else {
            onStartServerRequested();
        }
    });
    
    logMessage("Keyboard shortcuts enabled. Press F1 for list.", "[INFO] ");
}

void MainWindow::setupConnections()
{
    // Connection panel
    connect(connectionPanel, &ConnectionPanel::connectRequested,
            this, &MainWindow::onConnectRequested);
    connect(connectionPanel, &ConnectionPanel::disconnectRequested,
            this, &MainWindow::onDisconnectRequested);
    connect(connectionPanel, &ConnectionPanel::protocolChanged,
            this, &MainWindow::onClientProtocolChanged);
    connect(connectionPanel, &ConnectionPanel::httpPollingToggled,
            this, &MainWindow::onHttpPollingToggled);
    
    // Server panel
    connect(serverPanel, &ServerPanel::startServerRequested,
            this, &MainWindow::onStartServerRequested);
    connect(serverPanel, &ServerPanel::stopServerRequested,
            this, &MainWindow::onStopServerRequested);
    connect(serverPanel, &ServerPanel::protocolChanged,
            this, &MainWindow::onServerProtocolChanged);
    
    // Message panel
    connect(messagePanel, &MessagePanel::sendRequested,
            this, &MainWindow::onSendRequested);
    connect(messagePanel, &MessagePanel::formatChanged,
            this, &MainWindow::onFormatChanged);
    connect(messagePanel, &MessagePanel::loadMessageRequested,
            this, &MainWindow::onLoadMessageRequested);
    connect(messagePanel, &MessagePanel::saveMessageRequested,
            this, &MainWindow::onSaveMessageRequested);
    
    // Server panel send mode
    connect(serverPanel, &ServerPanel::sendModeChanged,
            this, &MainWindow::onSendModeChanged);
    
    // Display panel
    connect(displayPanel, &DisplayPanel::exportMessagesRequested,
            this, &MainWindow::onExportMessagesRequested);
    connect(displayPanel, &DisplayPanel::exportLogsRequested,
            this, &MainWindow::onExportLogsRequested);
}

// Connection handlers
void MainWindow::onConnectRequested()
{
    if (connectionPanel->isConnected()) {
        onDisconnectRequested();
        return;
    }
    
    if (!validateInputs()) {
        return;
    }
    
    QString protocol = connectionPanel->getProtocol();
    QString host = connectionPanel->getHost();
    int port = connectionPanel->getPort();
    DataFormatType format = messagePanel->getFormat();
    
    if (protocol == "TCP") {
        tcpClient->setFormat(format);
        tcpClient->connectToHost(host, static_cast<quint16>(port));
        logMessage(QString("Connecting to TCP server at %1:%2").arg(host).arg(port), "[CONNECT] ");
        // Status will be updated when connected() signal is emitted
    } else if (protocol == "UDP") {
        udpClient->setFormat(format);
        if (udpClient->connectToHost(host, static_cast<quint16>(port))) {
            connectionPanel->setConnectionState(true);
            logMessage(QString("Connected to UDP server at %1:%2").arg(host).arg(port), "[CONNECT] ");
        } else {
            QMessageBox::warning(this, "Connection Error", "Failed to connect via UDP");
        }
    } else if (protocol == "WebSocket") {
        wsClient->setFormat(format);
        wsClient->connectToServer(host);
        logMessage(QString("Connecting to WebSocket server at %1").arg(host), "[CONNECT] ");
        // Status will be updated when connected() signal is emitted
    } else if (protocol == "HTTP") {
        httpClient->setFormat(format);
        httpClient->setConnected(true);
        connectionPanel->setConnectionState(true);
        logMessage(QString("HTTP client ready for %1").arg(host), "[CONNECT] ");
        
        // Start polling if enabled
        if (connectionPanel->isHttpPollingEnabled()) {
            httpClient->startPolling(host, 2000);
            logMessage("HTTP long-polling enabled", "[HTTP] ");
        }
    }
}

void MainWindow::onDisconnectRequested()
{
    QString protocol = connectionPanel->getProtocol();
    
    if (protocol == "TCP") {
        tcpClient->disconnect();
    } else if (protocol == "UDP") {
        udpClient->disconnect();
    } else if (protocol == "WebSocket") {
        wsClient->disconnect();
    } else if (protocol == "HTTP") {
        httpClient->stopPolling();
        httpClient->setConnected(false);
    }
    
    connectionPanel->setConnectionState(false);
    updateStatus();
    logMessage(QString("Disconnected from %1 server").arg(protocol), "[DISCONNECT] ");
}

void MainWindow::onClientProtocolChanged(const QString &protocol)
{
    // Disconnect all clients when switching protocols
    if (tcpClient->isConnected()) tcpClient->disconnect();
    if (udpClient->isConnected()) udpClient->disconnect();
    if (wsClient->isConnected()) wsClient->disconnect();
    if (httpClient->isConnected()) {
        httpClient->stopPolling();
        httpClient->setConnected(false);
    }
    
    connectionPanel->setConnectionState(false);
    updateStatus();
    statusPanel->setProtocolInfo(protocol, serverPanel->getProtocol());
    logMessage(QString("Client protocol changed to %1").arg(protocol), "[INFO] ");
}

// Server handlers
void MainWindow::onStartServerRequested()
{
    QString protocol = serverPanel->getProtocol();
    int port = serverPanel->getPort();
    quint16 serverPort = static_cast<quint16>(port);
    DataFormatType format = messagePanel->getFormat();
    
    bool success = false;
    if (protocol == "TCP Server") {
        tcpServer->setFormat(format);
        success = tcpServer->startServer(serverPort);
    } else if (protocol == "UDP Server") {
        udpServer->setFormat(format);
        success = udpServer->startServer(serverPort);
    } else if (protocol == "WebSocket Server") {
        wsServer->setFormat(format);
        success = wsServer->startServer(serverPort);
    } else if (protocol == "HTTP Server") {
        httpServer->setFormat(format);
        success = httpServer->startServer(serverPort);
    }
    
    if (success) {
        serverPanel->setServerState(true);
        statusPanel->setServerStatus("Running", true);
        updateStatus();
        logMessage(QString("%1 started on port %2").arg(protocol).arg(port), "[SERVER] ");
    } else {
        QMessageBox::critical(this, "Server Error",
            QString("Failed to start %1 on port %2\n\nPlease check if the port is already in use.").arg(protocol).arg(port));
        logMessage(QString("Failed to start %1 on port %2").arg(protocol).arg(port), "[ERROR] ");
    }
}

void MainWindow::onStopServerRequested()
{
    QString protocol = serverPanel->getProtocol();
    
    if (protocol == "TCP Server") {
        tcpServer->stopServer();
    } else if (protocol == "UDP Server") {
        udpServer->stopServer();
    } else if (protocol == "WebSocket Server") {
        wsServer->stopServer();
    } else if (protocol == "HTTP Server") {
        httpServer->stopServer();
    }
    
    serverPanel->setServerState(false);
    serverPanel->clearClients();
    statusPanel->setServerStatus("Stopped", false);
    statusPanel->setClientCount(0);
    logMessage(QString("%1 stopped").arg(protocol), "[SERVER] ");
}

void MainWindow::onServerProtocolChanged(const QString &protocol)
{
    // Stop all servers when switching protocols
    if (tcpServer->isListening()) tcpServer->stopServer();
    if (udpServer->isListening()) udpServer->stopServer();
    if (wsServer->isListening()) wsServer->stopServer();
    if (httpServer->isListening()) httpServer->stopServer();
    
    // Clear client lists
    serverPanel->clearClients();
    
    serverPanel->setServerState(false);
    updateStatus();
    statusPanel->setProtocolInfo(connectionPanel->getProtocol(), protocol);
    logMessage(QString("Server protocol changed to %1").arg(protocol), "[INFO] ");
}

void MainWindow::onSendModeChanged(const QString &mode)
{
    updateSendButtonState();
    logMessage(QString("Send mode changed to %1").arg(mode), "[INFO] ");
}

/**
 * @brief Handles message send request - Complete send flow
 * 
 * Flow:
 * 1. Receives sendRequested() signal from MessagePanel
 * 2. Gets message text and format from MessagePanel
 * 3. Validates message is not empty
 * 4. Validates input format (JSON syntax, hex format, etc.)
 * 5. Parses input string to QVariant using DataMessage::parseInput()
 *    - JSON: QJsonDocument
 *    - XML/CSV/TEXT: QString
 *    - BINARY/HEX: QByteArray
 * 6. Creates DataMessage object with format and parsed data
 * 7. Checks send mode (Client or Server)
 * 
 * 8. If Client mode:
 *    - Gets protocol from ConnectionPanel
 *    - Calls appropriate network client's sendMessage(msg)
 *    - Network component serializes DataMessage to bytes
 *    - Bytes sent over network (asynchronous)
 * 
 * 9. If Server mode (Broadcast or Selected):
 *    - Gets protocol from ServerPanel
 *    - Broadcast: Sends to all connected clients
 *    - Selected: Sends to specific client
 * 
 * 10. Updates DisplayPanel with sent message
 * 11. Logs the action
 * 12. Saves to database via MessageHistoryManager
 * 
 * @note All network operations are asynchronous
 * @note Message serialization format depends on DataFormatType
 * @see DataMessage::serialize() for serialization details
 */
// Message handlers
void MainWindow::onSendRequested()
{
    QString messageText = messagePanel->getMessage();
    if (messageText.isEmpty()) {
        QMessageBox::warning(this, "Error", "Message cannot be empty");
        return;
    }
    
    DataFormatType format = messagePanel->getFormat();
    if (!DataMessage::validateInput(messageText, format)) {
        QMessageBox::warning(this, "Input Error", "Invalid input for selected format");
        return;
    }
    
    QVariant parsedData = DataMessage::parseInput(messageText, format);
    DataMessage msg(format, parsedData);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    
    QString sendMode = serverPanel->getSendMode();
    
    // SERVER-SIDE SENDING
    if (sendMode.contains("Broadcast") || sendMode.contains("Selected")) {
        QString serverProto = serverPanel->getProtocol();
        
        if (sendMode.contains("Broadcast")) {
            // BROADCAST TO ALL CLIENTS
            if (serverProto == "TCP Server" && tcpServer->isListening()) {
                tcpServer->sendToAll(msg);
                displayPanel->appendSentMessage(QString("[%1] → TCP Server Broadcast:\n%2").arg(timestamp, messageText));
                logMessage("TCP Server broadcast to all clients", "[SEND] ");
                historyManager.saveMessage("sent", "TCP", "broadcast", serverPanel->getPort(), msg);
            } else if (serverProto == "WebSocket Server" && wsServer->isListening()) {
                wsServer->sendToAll(msg);
                displayPanel->appendSentMessage(QString("[%1] → WebSocket Server Broadcast:\n%2").arg(timestamp, messageText));
                logMessage("WebSocket Server broadcast to all clients", "[SEND] ");
                historyManager.saveMessage("sent", "WebSocket", "broadcast", serverPanel->getPort(), msg);
            } else if (serverProto == "UDP Server" && udpServer->isListening()) {
                QMessageBox::information(this, "Info", "UDP server replies automatically when it receives messages.");
            } else if (serverProto == "HTTP Server" && httpServer->isListening()) {
                httpServer->queueMessageForAll(msg);
                displayPanel->appendSentMessage(QString("[%1] → HTTP Server Broadcast (queued):\n%2").arg(timestamp, messageText));
                logMessage("HTTP Server queued broadcast", "[SEND] ");
                historyManager.saveMessage("sent", "HTTP", "broadcast", serverPanel->getPort(), msg);
            } else {
                QMessageBox::warning(this, "Error", "Server not listening");
            }
        } else {
            // SEND TO SELECTED CLIENT
            QString targetClient = serverPanel->getTargetClient();
            if (targetClient.isEmpty()) {
                QMessageBox::warning(this, "Error", "No client selected");
                return;
            }
            
            bool sent = false;
            if (serverProto == "TCP Server" && tcpServer->isListening()) {
                QTcpSocket* client = tcpServer->findClientByAddress(targetClient);
                if (client) {
                    tcpServer->sendToClient(client, msg);
                    sent = true;
                }
            } else if (serverProto == "WebSocket Server" && wsServer->isListening()) {
                QWebSocket* client = wsServer->findClientByAddress(targetClient);
                if (client) {
                    wsServer->sendToClient(client, msg);
                    sent = true;
                }
            } else if (serverProto == "UDP Server" && udpServer->isListening()) {
                QStringList parts = targetClient.split(":");
                if (parts.size() == 2) {
                    QHostAddress addr(parts[0]);
                    quint16 port = parts[1].toUShort();
                    udpServer->sendTo(addr, port, msg);
                    sent = true;
                }
            } else if (serverProto == "HTTP Server" && httpServer->isListening()) {
                QTcpSocket* client = httpServer->findClientByAddress(targetClient);
                if (client) {
                    httpServer->queueMessageForClient(client, msg);
                    sent = true;
                }
            }
            
            if (sent) {
                displayPanel->appendSentMessage(QString("[%1] → %2 to %3:\n%4").arg(timestamp, serverProto, targetClient, messageText));
                logMessage(QString("%1 sent to %2").arg(serverProto, targetClient), "[SEND] ");
                // Strip " Server" suffix for database (CHECK constraint requires TCP/UDP/WebSocket/HTTP only)
                QString dbProtocol = serverProto;
                dbProtocol.remove(" Server");
                historyManager.saveMessage("sent", dbProtocol, targetClient, serverPanel->getPort(), msg);
            } else {
                QMessageBox::warning(this, "Error", "Client not found or disconnected");
            }
        }
        return;
    }
    
    // CLIENT-SIDE SENDING
    QString proto = connectionPanel->getProtocol();
    
    if (proto == "HTTP") {
        QString url = connectionPanel->getHost();
        if (!url.startsWith("http://") && !url.startsWith("https://")) {
            url = "http://" + url;
        }
        
        QString methodStr = connectionPanel->getHttpMethod();
        HttpClient::Method method = HttpClient::POST;
        if (methodStr == "GET") method = HttpClient::GET;
        else if (methodStr == "POST") method = HttpClient::POST;
        else if (methodStr == "PUT") method = HttpClient::PUT;
        else if (methodStr == "DELETE") method = HttpClient::DELETE;
        else if (methodStr == "PATCH") method = HttpClient::PATCH;
        else if (methodStr == "HEAD") method = HttpClient::HEAD;
        else if (methodStr == "OPTIONS") method = HttpClient::OPTIONS;
        
        httpClient->sendRequest(url, method, msg);
        displayPanel->appendSentMessage(QString("[%1] → HTTP %2 to %3:\n%4").arg(timestamp, methodStr, url, messageText));
        logMessage(QString("Sent via HTTP %1").arg(methodStr), "[SEND] ");
        historyManager.saveMessage("sent", "HTTP", url, 0, msg);
    } else if (proto == "WebSocket" && wsClient->isConnected()) {
        wsClient->sendMessage(msg);
        displayPanel->appendSentMessage(QString("[%1] → WebSocket to %2:\n%3").arg(timestamp, connectionPanel->getHost(), messageText));
        logMessage("Sent via WebSocket", "[SEND] ");
        historyManager.saveMessage("sent", "WebSocket", connectionPanel->getHost(), 0, msg);
    } else if (proto == "TCP" && tcpClient->isConnected()) {
        tcpClient->sendMessage(msg);
        displayPanel->appendSentMessage(QString("[%1] → TCP to %2:%3:\n%4").arg(timestamp, connectionPanel->getHost()).arg(connectionPanel->getPort()).arg(messageText));
        logMessage("Sent via TCP", "[SEND] ");
        historyManager.saveMessage("sent", "TCP", connectionPanel->getHost(), connectionPanel->getPort(), msg);
    } else if (proto == "UDP" && udpClient->isConnected()) {
        udpClient->sendMessage(msg);
        displayPanel->appendSentMessage(QString("[%1] → UDP to %2:%3:\n%4").arg(timestamp, connectionPanel->getHost()).arg(connectionPanel->getPort()).arg(messageText));
        logMessage("Sent via UDP", "[SEND] ");
        historyManager.saveMessage("sent", "UDP", connectionPanel->getHost(), connectionPanel->getPort(), msg);
    } else {
        QMessageBox::warning(this, "Error", "Not connected");
    }
}

void MainWindow::onFormatChanged(const QString &format)
{
    DataFormatType formatType = messagePanel->getFormat();
    
    // Synchronize format across all network components
    tcpClient->setFormat(formatType);
    tcpServer->setFormat(formatType);
    udpClient->setFormat(formatType);
    udpServer->setFormat(formatType);
    wsClient->setFormat(formatType);
    wsServer->setFormat(formatType);
    httpClient->setFormat(formatType);
    httpServer->setFormat(formatType);
    
    logMessage(QString("Data format changed to %1").arg(format), "[INFO] ");
}

void MainWindow::onLoadMessageRequested()
{
    QString defaultLoc = FileManager::getDefaultSaveLocation();
    QString filter = "JSON Files (*.json);;XML Files (*.xml);;CSV Files (*.csv);;Text Files (*.txt);;All Files (*)";
    QString filename = QFileDialog::getOpenFileName(this, "Load Message", defaultLoc, filter);
    
    if (!filename.isEmpty()) {
        DataFormatType format = messagePanel->getFormat();
        QString content = FileManager::loadMessageFromFile(filename, format);
        if (!content.isEmpty()) {
            messagePanel->setMessage(content);
            QString formatStr = messagePanel->getDataFormat();
            QMessageBox::information(this, "Success", formatStr + " file loaded successfully");
            logMessage(QString("Message loaded from %1").arg(filename), "[FILE] ");
        } else {
            QMessageBox::warning(this, "Error", "Failed to load message from file");
        }
    }
}

void MainWindow::onSaveMessageRequested()
{
    QString defaultLoc = FileManager::getDefaultSaveLocation();
    QString formatStr = messagePanel->getDataFormat().toLower();
    QString defaultName = "/message." + formatStr;
    QString filter = "JSON Files (*.json);;XML Files (*.xml);;CSV Files (*.csv);;Text Files (*.txt);;All Files (*)";
    QString filename = QFileDialog::getSaveFileName(this, "Save Message", defaultLoc + defaultName, filter);
    
    if (!filename.isEmpty()) {
        QString content = messagePanel->getMessage();
        DataFormatType format = messagePanel->getFormat();
        if (FileManager::saveMessageToFile(content, filename, format)) {
            QString displayFormat = messagePanel->getDataFormat();
            QMessageBox::information(this, "Success", displayFormat + " file saved successfully");
            logMessage(QString("Message saved to %1").arg(filename), "[FILE] ");
        } else {
            QMessageBox::warning(this, "Error", "Failed to save message to file");
        }
    }
}

// Display handlers
void MainWindow::onExportMessagesRequested()
{
    QString defaultLoc = FileManager::getDefaultSaveLocation();
    QString filename = QFileDialog::getSaveFileName(this, "Export Messages", 
        defaultLoc + "/messages.json", 
        "JSON Files (*.json);;Text Files (*.txt);;CSV Files (*.csv);;All Files (*)");
    
    if (!filename.isEmpty()) {
        // Detect format from file extension
        QString format = "json";
        if (filename.endsWith(".txt")) format = "txt";
        else if (filename.endsWith(".csv")) format = "csv";
        
        if (ExportManager::exportMessages(receivedMessages, format, filename)) {
            QMessageBox::information(this, "Success", "Messages exported successfully to: " + filename);
            logMessage(QString("Messages exported to %1").arg(filename), "[EXPORT] ");
        } else {
            QMessageBox::warning(this, "Error", "Failed to export messages");
        }
    }
}

void MainWindow::onExportLogsRequested()
{
    QString defaultLoc = FileManager::getDefaultSaveLocation();
    QString filename = QFileDialog::getSaveFileName(this, "Export Logs", 
        defaultLoc + "/logs.txt", 
        "Text Files (*.txt);;CSV Files (*.csv);;All Files (*)");
    
    if (!filename.isEmpty()) {
        // Detect format from file extension
        QString format = filename.endsWith(".csv") ? "csv" : "txt";
        
        QStringList logs;
        logs << displayPanel->getLogs();
        if (ExportManager::exportLogs(logs, format, filename)) {
            QMessageBox::information(this, "Success", "Logs exported successfully to: " + filename);
            logMessage(QString("Logs exported to %1").arg(filename), "[EXPORT] ");
        } else {
            QMessageBox::warning(this, "Error", "Failed to export logs");
        }
    }
}

/**
 * @brief Handles incoming data from all network components
 * 
 * @param msg The received DataMessage (already deserialized)
 * @param source Source address/identifier (e.g., "127.0.0.1:5000")
 * @param timestamp Timestamp when message was received
 * 
 * Flow:
 * 1. Receives messageReceived() signal from network component
 *    (TcpClient, TcpServer, UdpClient, UdpServer, WebSocketClient, 
 *     WebSocketServer, HttpClient, or HttpServer)
 * 
 * 2. Network component has already:
 *    - Read bytes from socket
 *    - Deserialized bytes to DataMessage
 *    - Created timestamp
 *    - Extracted source address
 * 
 * 3. Determines protocol from sender object:
 *    - tcpClient/tcpServer → "TCP"
 *    - udpClient/udpServer → "UDP"
 *    - wsClient/wsServer → "WebSocket"
 *    - httpClient/httpServer → "HTTP"
 * 
 * 4. Converts DataMessage to display string using toDisplayString()
 *    - JSON: Pretty-printed (indented)
 *    - XML/CSV/TEXT: As-is
 *    - BINARY: Hex representation with size
 *    - HEX: Hex string
 * 
 * 5. Appends to DisplayPanel (received messages area)
 * 
 * 6. Logs the received message
 * 
 * 7. Parses source string to extract host and port
 * 
 * 8. Saves to database via MessageHistoryManager::saveMessage()
 * 
 * 9. Updates status panel
 * 
 * @note This slot is connected to all 8 network components
 * @note DataMessage is already deserialized by the network component
 * @note All operations happen in main thread (thread-safe)
 * @see TcpClient::onReadyRead() for deserialization example
 */
// Network event handlers
void MainWindow::onDataReceived(const DataMessage &msg, const QString &source, const QString &timestamp)
{
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
    
    QString displayText = msg.toDisplayString();
    QString message = QString("[%1] ← %2 from %3:\n%4\n")
                     .arg(timestamp, protocol, source, displayText);
    
    // Determine if this is from a client or server
    bool isClientMessage = (senderObj == tcpClient || senderObj == udpClient || 
                           senderObj == wsClient || senderObj == httpClient);
    bool isServerMessage = (senderObj == tcpServer || senderObj == udpServer || 
                           senderObj == wsServer || senderObj == httpServer);
    
    // Add to appropriate tabs
    if (isClientMessage) {
        displayPanel->appendReceivedMessage(message, false);
        logMessage(QString("Client received %1 message from %2").arg(protocol, source), "[RECV] ");
    } else if (isServerMessage) {
        displayPanel->appendReceivedMessage(message, true);
        logMessage(QString("Server received %1 message from %2").arg(protocol, source), "[RECV] ");
    }
    
    // Save to history database
    QString host = source;
    int port = 0;
    
    // Parse host and port from source
    if (protocol == "HTTP") {
        if (source.startsWith("http://") || source.startsWith("https://")) {
            QUrl url(source.split(" ").first());
            host = url.host();
            port = url.port(80);
        } else {
            QStringList parts = source.split(" ").first().split(":");
            if (parts.size() >= 2) {
                port = parts.last().toInt();
                parts.removeLast();
                host = parts.join(":");
            }
        }
    } else {
        QStringList parts = source.split(":");
        if (parts.size() >= 2) {
            port = parts.last().toInt();
            parts.removeLast();
            host = parts.join(":");
        }
    }
    
    if (!historyManager.saveMessage("received", protocol, host, port, msg, source)) {
        logMessage("Failed to save message to history", "[WARN] ");
    }
    
    // Save settings on successful receive
    saveSettings();
}

void MainWindow::onClientConnected(const QString &clientInfo)
{
    serverPanel->addClient(clientInfo);
    statusPanel->setClientCount(serverPanel->getClientCount());
    
    updateSendButtonState();
    logMessage(QString("Client connected: %1").arg(clientInfo), "[CLIENT] ");
}

void MainWindow::onClientDisconnected(const QString &clientInfo)
{
    serverPanel->removeClient(clientInfo);
    statusPanel->setClientCount(serverPanel->getClientCount());
    
    updateSendButtonState();
    logMessage(QString("Client disconnected: %1").arg(clientInfo), "[CLIENT] ");
}

void MainWindow::onNetworkError(const QString &error)
{
    logMessage(QString("Network error: %1").arg(error), "[ERROR] ");
    QMessageBox::critical(this, "Network Error", error);
}

void MainWindow::onHttpPollingToggled(bool enabled)
{
    if (enabled && httpClient->isConnected()) {
        QString url = connectionPanel->getHost();
        httpClient->startPolling(url, 2000);
        logMessage("HTTP long-polling enabled", "[HTTP] ");
    } else {
        httpClient->stopPolling();
        logMessage("HTTP long-polling disabled", "[HTTP] ");
    }
}

// Theme handlers
void MainWindow::onThemeChanged()
{
    // Apply theme to the entire application
    ThemeManager::instance().applyTheme(this);
    
    // Update menu checkmarks
    ThemeManager::Theme theme = ThemeManager::instance().currentTheme();
    if (theme == ThemeManager::Theme::Light) {
        lightModeAction->setChecked(true);
    } else if (theme == ThemeManager::Theme::Dark) {
        darkModeAction->setChecked(true);
    } else {
        autoModeAction->setChecked(true);
    }
    
    // Update status bar to reflect theme change
    updateStatus();
}

void MainWindow::onToggleLightMode()
{
    ThemeManager::instance().setTheme(ThemeManager::Theme::Light);
    logMessage("Theme changed to Light Mode", "[THEME] ");
}

void MainWindow::onToggleDarkMode()
{
    ThemeManager::instance().setTheme(ThemeManager::Theme::Dark);
    logMessage("Theme changed to Dark Mode", "[THEME] ");
}

void MainWindow::onToggleAutoMode()
{
    ThemeManager::instance().setTheme(ThemeManager::Theme::Auto);
    logMessage("Theme changed to System Default", "[THEME] ");
}

// Utility
void MainWindow::showShortcutsHelp()
{
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("Keyboard Shortcuts");
    dialog->setMinimumWidth(500);
    dialog->setMinimumHeight(400);
    
    auto *layout = new QVBoxLayout(dialog);
    
    auto *table = new QTableWidget(11, 2);
    table->setHorizontalHeaderLabels({"Shortcut", "Action"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    QStringList shortcuts = {
        "F1", "Show this help",
        "Ctrl+Return", "Send message",
        "Ctrl+K", "Connect/Disconnect",
        "Ctrl+O", "Load message from file",
        "Ctrl+S", "Save message to file",
        "Ctrl+L", "Clear all messages",
        "Ctrl+E", "Export messages",
        "Ctrl+Shift+E", "Export logs",
        "Ctrl+R", "Start/Stop server",
        "Ctrl+Q", "Quit application",
        "Esc", "Close dialogs"
    };
    
    for (int i = 0; i < shortcuts.size(); i += 2) {
        int row = i / 2;
        table->setItem(row, 0, new QTableWidgetItem(shortcuts[i]));
        table->setItem(row, 1, new QTableWidgetItem(shortcuts[i + 1]));
    }
    
    table->resizeColumnsToContents();
    layout->addWidget(table);
    
    auto *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    dialog->exec();
}

bool MainWindow::validateInputs()
{
    QString host = connectionPanel->getHost();
    int port = connectionPanel->getPort();
    QString protocol = connectionPanel->getProtocol();
    
    if (host.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Host cannot be empty");
        return false;
    }
    
    // Skip port validation for WebSocket and HTTP (they use full URLs)
    if (protocol != "WebSocket" && protocol != "HTTP") {
        if (port < 1 || port > 65535) {
            QMessageBox::warning(this, "Invalid Input", "Port must be between 1 and 65535");
            return false;
        }
    }
    
    return true;
}

void MainWindow::updateStatus()
{
    updateClientStatus();
    updateServerStatus();
    updateSendButtonState();
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    bool anyClientConnected = tcpClient->isConnected() || udpClient->isConnected() || 
                              wsClient->isConnected() || httpClient->isConnected();
    bool anyServerListening = tcpServer->isListening() || udpServer->isListening() || 
                              wsServer->isListening() || httpServer->isListening();
    
    QString sendStatus = anyClientConnected ? 
        QString("TX: %1:%2").arg(connectionPanel->getHost()).arg(connectionPanel->getPort()) : "TX: Idle";
    QString recvStatus = anyServerListening ? 
        QString("RX: Port %1").arg(serverPanel->getPort()) : "RX: Idle";
    QString themeStatus = QString("UI: %1").arg(ThemeManager::instance().getThemeName());
    QString status = QString("%1 | %2 | %3").arg(sendStatus, recvStatus, themeStatus);
    
    statusPanel->setStatusMessage(status);
}

void MainWindow::updateClientStatus()
{
    bool anyConnected = tcpClient->isConnected() || udpClient->isConnected() || 
                        wsClient->isConnected() || httpClient->isConnected();
    
    QString protocol = connectionPanel->getProtocol();
    statusPanel->setClientStatus(anyConnected ? QString("Connected (%1)").arg(protocol) : "Disconnected", anyConnected);
    
    if (anyConnected) {
        connectionPanel->setConnectionState(true);
    }
}

void MainWindow::updateServerStatus()
{
    bool anyListening = tcpServer->isListening() || udpServer->isListening() || 
                        wsServer->isListening() || httpServer->isListening();
    
    QString protocol = serverPanel->getProtocol();
    statusPanel->setServerStatus(anyListening ? QString("Running (%1)").arg(protocol) : "Stopped", anyListening);
}

void MainWindow::updateSendButtonState()
{
    bool clientConnected = tcpClient->isConnected() || udpClient->isConnected() || 
                          wsClient->isConnected() || httpClient->isConnected();
    bool serverHasClients = serverPanel->getClientCount() > 0;
    bool serverListening = tcpServer->isListening() || udpServer->isListening() || 
                          wsServer->isListening() || httpServer->isListening();
    
    QString sendMode = serverPanel->getSendMode();
    
    bool canSend = false;
    if (sendMode.contains("Client")) {
        canSend = clientConnected || (connectionPanel->getProtocol() == "HTTP");
    } else if (sendMode.contains("Broadcast")) {
        canSend = serverListening && serverHasClients;
    } else if (sendMode.contains("Selected")) {
        canSend = serverListening && !serverPanel->getTargetClient().isEmpty();
    }
    
    messagePanel->setSendButtonEnabled(canSend);
}

void MainWindow::logMessage(const QString &message, const QString &prefix)
{
    displayPanel->appendLogMessage(prefix + message);
    statusPanel->setStatusMessage(message);
}

void MainWindow::saveSettings()
{
    QSettings settings("CommLink", "ModularGUI");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("clientProtocol", connectionPanel->getProtocol());
    settings.setValue("clientHost", connectionPanel->getHost());
    settings.setValue("clientPort", connectionPanel->getPort());
    settings.setValue("serverProtocol", serverPanel->getProtocol());
    settings.setValue("serverPort", serverPanel->getPort());
    settings.setValue("dataFormat", messagePanel->getDataFormat());
}

void MainWindow::loadSettings()
{
    QSettings settings("CommLink", "ModularGUI");
    restoreGeometry(settings.value("geometry").toByteArray());
    
    if (settings.contains("clientProtocol")) {
        connectionPanel->setProtocol(settings.value("clientProtocol").toString());
    }
    if (settings.contains("clientHost")) {
        connectionPanel->setHost(settings.value("clientHost").toString());
    }
    if (settings.contains("clientPort")) {
        connectionPanel->setPort(settings.value("clientPort").toInt());
    }
    if (settings.contains("serverProtocol")) {
        serverPanel->setProtocol(settings.value("serverProtocol").toString());
    }
    if (settings.contains("serverPort")) {
        serverPanel->setPort(settings.value("serverPort").toInt());
    }
    if (settings.contains("dataFormat")) {
        messagePanel->setDataFormat(settings.value("dataFormat").toString());
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Disconnect all clients
    tcpClient->disconnect();
    udpClient->disconnect();
    wsClient->disconnect();
    httpClient->stopPolling();
    httpClient->setConnected(false);
    
    // Stop all servers
    tcpServer->stopServer();
    udpServer->stopServer();
    wsServer->stopServer();
    httpServer->stopServer();
    
    // Save settings
    saveSettings();
    ThemeManager::instance().saveSettings();
    
    event->accept();
    QApplication::quit();
}
