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
    void onConnectRequested();
    void onDisconnectRequested();
    void onClientProtocolChanged(const QString &protocol);
    
    // Server handlers
    void onStartServerRequested();
    void onStopServerRequested();
    void onServerProtocolChanged(const QString &protocol);
    void onSendModeChanged(const QString &mode);
    
    // Message handlers
    void onSendRequested();
    void onFormatChanged(const QString &format);
    void onLoadMessageRequested();
    void onSaveMessageRequested();
    
    // Display handlers
    void onExportMessagesRequested();
    void onExportLogsRequested();
    
    // Network event handlers
    void onDataReceived(const DataMessage &msg, const QString &source, const QString &timestamp);
    void onClientConnected(const QString &clientInfo);
    void onClientDisconnected(const QString &clientInfo);
    void onNetworkError(const QString &error);
    void onHttpPollingToggled(bool enabled);
    
    // Theme handlers
    void onThemeChanged();
    void onToggleLightMode();
    void onToggleDarkMode();
    void onToggleAutoMode();
    
    // Utility
    void showShortcutsHelp();

private:
    void setupUI();
    void setupMenuBar();
    void setupShortcuts();
    void setupConnections();
    void initializeNetworkComponents();
    void saveSettings();
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
