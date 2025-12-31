#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QAction>
#include <QtCore/QSettings>
#include <QtGui/QCloseEvent>

// Network components
#include "../network/tcpclient.h"
#include "../network/tcpserver.h"
#include "../network/udpclient.h"
#include "../network/udpserver.h"
#include "../network/websocketclient.h"
#include "../network/websocketserver.h"
#include "../network/httpclient.h"
#include "../network/httpserver.h"

// Core components
#include "../core/filemanager.h"
#include "../core/exportmanager.h"
#include "../core/messagehistorymanager.h"
#include "../core/logger.h"
#include "thememanager.h"

// UI Panels
#include "connectionpanel.h"
#include "serverpanel.h"
#include "messagepanel.h"
#include "displaypanel.h"
#include "statuspanel.h"

/**
 * @brief Main application window with modular UI components
 * 
 * Orchestrates all UI panels and manages network communication.
 * This is a complete rewrite of the monolithic GUI with a modular architecture.
 * 
 * @section flow_sec Code Flow Overview
 * 
 * The MainWindow follows a signal-slot orchestration pattern:
 * 
 * @subsection startup_flow Application Startup Flow
 * 
 * 1. Constructor is called from main()
 * 2. initializeNetworkComponents() - Creates all 8 network objects (4 clients + 4 servers)
 * 3. setupUI() - Creates all 5 UI panels
 * 4. setupConnections() - Wires UI panel signals to MainWindow slots
 * 5. Network component signals are already connected in initializeNetworkComponents()
 * 6. loadSettings() - Restores window state
 * 
 * @subsection connection_flow Connection Flow
 * 
 * User clicks "Connect" → ConnectionPanel emits connectRequested() → 
 * MainWindow::onConnectRequested() → Network component::connectToHost() →
 * Network component emits connected() → MainWindow::updateStatus() → UI updated
 * 
 * @subsection send_flow Message Sending Flow
 * 
 * User types message → Clicks "Send" → MessagePanel emits sendRequested() →
 * MainWindow::onSendRequested() → Validates input → Parses to DataMessage →
 * Network component::sendMessage() → Serializes → Network transmission →
 * UI updated + Database saved
 * 
 * @subsection receive_flow Message Receiving Flow
 * 
 * Network data arrives → Network component::onReadyRead() → Deserializes →
 * Network component emits messageReceived() → MainWindow::onDataReceived() →
 * DisplayPanel updated + Database saved + Logged
 * 
 * @see @ref CODE_FLOW.md for detailed flow documentation
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // Connection handlers
    /**
     * @brief Handles client connection request from ConnectionPanel
     * 
     * Flow:
     * 1. Receives connectRequested() signal from ConnectionPanel
     * 2. Validates user inputs (host, port)
     * 3. Gets protocol, host, port from ConnectionPanel
     * 4. Calls appropriate network client's connectToHost()
     * 5. Connection happens asynchronously
     * 6. When connected, network component emits connected() signal
     * 7. MainWindow::updateStatus() is called to update UI
     * 
     * @note All network connections are asynchronous and non-blocking
     */
    void onConnectRequested();
    
    /**
     * @brief Handles client disconnection request
     * 
     * @flow
     * 1. Receives disconnectRequested() signal
     * 2. Calls network client's disconnect() method
     * 3. Updates UI connection state
     */
    void onDisconnectRequested();
    
    /**
     * @brief Handles protocol change in ConnectionPanel
     * 
     * Updates UI fields visibility based on selected protocol
     */
    void onClientProtocolChanged(const QString &protocol);
    
    // Server handlers
    /**
     * @brief Handles server start request from ServerPanel
     * 
     * @flow
     * 1. Receives startServerRequested() signal
     * 2. Gets protocol and port from ServerPanel
     * 3. Calls appropriate network server's startServer(port)
     * 4. Server starts listening on specified port
     * 5. Updates UI to show server is running
     */
    void onStartServerRequested();
    
    /**
     * @brief Handles server stop request
     * 
     * Stops the server and updates UI
     */
    void onStopServerRequested();
    
    /**
     * @brief Handles server protocol change
     */
    void onServerProtocolChanged(const QString &protocol);
    
    /**
     * @brief Handles send mode change (Client/Server Broadcast/Selected)
     */
    void onSendModeChanged(const QString &mode);
    
    // Message handlers
    /**
     * @brief Handles message send request from MessagePanel
     * 
     * Flow:
     * 1. Receives sendRequested() signal from MessagePanel
     * 2. Gets message text and format from MessagePanel
     * 3. Validates input format (JSON, XML, etc.)
     * 4. Parses input string to QVariant using DataMessage::parseInput()
     * 5. Creates DataMessage object with format and parsed data
     * 6. Checks send mode (Client or Server)
     * 7. If Client mode:
     *    - Gets protocol from ConnectionPanel
     *    - Calls appropriate network client's sendMessage()
     *    - Network component serializes DataMessage to bytes
     *    - Bytes sent over network
     * 8. If Server mode:
     *    - Broadcasts to all clients or sends to selected client
     * 9. Updates DisplayPanel with sent message
     * 10. Logs the action
     * 11. Saves to database via MessageHistoryManager
     * 
     * @note Message serialization format depends on DataFormatType
     * @see DataMessage::serialize() for serialization details
     */
    void onSendRequested();
    
    /**
     * @brief Handles data format change
     * 
     * Synchronizes format across all network components
     */
    void onFormatChanged(const QString &format);
    
    /**
     * @brief Handles load message from file request
     */
    void onLoadMessageRequested();
    
    /**
     * @brief Handles save message to file request
     */
    void onSaveMessageRequested();
    
    // Display handlers
    /**
     * @brief Handles export messages request
     */
    void onExportMessagesRequested();
    
    /**
     * @brief Handles export logs request
     */
    void onExportLogsRequested();
    
    // Network event handlers
    /**
     * @brief Handles incoming data from network components
     * 
     * @param msg The received DataMessage
     * @param source Source address/identifier (e.g., "127.0.0.1:5000")
     * @param timestamp Timestamp when message was received
     * 
     * Flow:
     * 1. Receives messageReceived() signal from network component
     * 2. Network component has already deserialized bytes to DataMessage
     * 3. Determines protocol from sender object
     * 4. Converts DataMessage to display string using toDisplayString()
     * 5. Appends to DisplayPanel (received messages area)
     * 6. Logs the received message
     * 7. Parses source to extract host and port
     * 8. Saves to database via MessageHistoryManager
     * 9. Updates status panel
     * 
     * @note This slot is connected to all network components (8 total)
     * @note DataMessage is already deserialized by the network component
     * @see TcpClient::onReadyRead() for deserialization example
     */
    void onDataReceived(const DataMessage &msg, const QString &source, const QString &timestamp);
    
    /**
     * @brief Handles client connection event from servers
     * 
     * Called when a server receives a new client connection
     */
    void onClientConnected(const QString &clientInfo);
    
    /**
     * @brief Handles client disconnection event from servers
     */
    void onClientDisconnected(const QString &clientInfo);
    
    /**
     * @brief Handles network errors from all network components
     * 
     * Displays error in status panel and logs it
     */
    void onNetworkError(const QString &error);
    
    /**
     * @brief Handles HTTP polling toggle
     */
    void onHttpPollingToggled(bool enabled);
    
    // Theme handlers
    void onThemeChanged();
    void onToggleLightMode();
    void onToggleDarkMode();
    void onToggleAutoMode();
    
    // Utility
    void showShortcutsHelp();

private:
    /**
     * @brief Sets up the user interface
     * 
     * Creates all UI panels and arranges them in a splitter layout:
     * - Left panel: ConnectionPanel, ServerPanel, MessagePanel
     * - Right panel: DisplayPanel, StatusPanel
     */
    void setupUI();
    
    /**
     * @brief Sets up the menu bar
     * 
     * Creates View menu with theme options and Help menu
     */
    void setupMenuBar();
    
    /**
     * @brief Sets up keyboard shortcuts
     * 
     * Configures shortcuts like Ctrl+Return for send, Ctrl+K for connect
     */
    void setupShortcuts();
    
    /**
     * @brief Wires UI panel signals to MainWindow slots
     * 
     * This function establishes all signal-slot connections between:
     * - UI Panels → MainWindow slots
     * 
     * Connections made:
     * - ConnectionPanel::connectRequested → MainWindow::onConnectRequested
     * - MessagePanel::sendRequested → MainWindow::onSendRequested
     * - ServerPanel::startServerRequested → MainWindow::onStartServerRequested
     * - DisplayPanel::exportMessagesRequested → MainWindow::onExportMessagesRequested
     * - And more...
     * 
     * @note Network component signals are connected in initializeNetworkComponents()
     */
    void setupConnections();
    
    /**
     * @brief Initializes all network components
     * 
     * Flow:
     * 1. Creates all 8 network objects (4 clients + 4 servers):
     *    - TcpClient, TcpServer
     *    - UdpClient, UdpServer
     *    - WebSocketClient, WebSocketServer
     *    - HttpClient, HttpServer
     * 2. Connects network component signals to MainWindow slots:
     *    - connected() → updateStatus()
     *    - messageReceived() → onDataReceived()
     *    - errorOccurred() → onNetworkError()
     *    - clientConnected() → onClientConnected()
     *    - clientDisconnected() → onClientDisconnected()
     * 
     * @note All network components are created with MainWindow as parent
     * @note Signals are connected immediately, ready for use
     */
    void initializeNetworkComponents();
    
    /**
     * @brief Saves application settings
     * 
     * Saves window geometry, theme preference, etc. to QSettings
     */
    void saveSettings();
    
    /**
     * @brief Loads application settings
     * 
     * Restores window geometry, theme, etc. from QSettings
     */
    void loadSettings();
    
    bool validateInputs();
    void updateStatus();
    void updateClientStatus();
    void updateServerStatus();
    void updateSendButtonState();
    void updateStatusBar();
    void logMessage(const QString &message, const QString &prefix = "");

    // UI Panels
    ConnectionPanel *connectionPanel;
    ServerPanel *serverPanel;
    MessagePanel *messagePanel;
    DisplayPanel *displayPanel;
    StatusPanel *statusPanel;

    // Menu components
    QMenuBar *menuBar;
    QAction *lightModeAction;
    QAction *darkModeAction;
    QAction *autoModeAction;

    // Network components
    TcpClient *tcpClient;
    TcpServer *tcpServer;
    UdpClient *udpClient;
    UdpServer *udpServer;
    WebSocketClient *wsClient;
    WebSocketServer *wsServer;
    HttpClient *httpClient;
    HttpServer *httpServer;

    // Business logic
    MessageHistoryManager historyManager;
    QList<DataMessage> receivedMessages;

    // Constants
    static constexpr int DEFAULT_WIDTH = 1400;
    static constexpr int DEFAULT_HEIGHT = 800;
    static constexpr int MIN_WIDTH = 1000;
    static constexpr int MIN_HEIGHT = 600;
};
