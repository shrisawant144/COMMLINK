#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QListWidget>
#include <QtGui/QIntValidator>
#include <QtGui/QCloseEvent>
#include <QtCore/QJsonDocument>
#include <QtCore/QSettings>
#include "../core/dataformat.h"
#include "../network/tcpclient.h"
#include "../network/tcpserver.h"
#include "../network/udpclient.h"
#include "../network/udpserver.h"
#include "../network/websocketclient.h"
#include "../network/websocketserver.h"
#include "../network/httpclient.h"
#include "../network/httpserver.h"
#include "../core/filemanager.h"
#include "../core/exportmanager.h"
#include "../core/messagehistorymanager.h"
#include "thememanager.h"
#include "../core/logger.h"

class CommLinkGUI : public QWidget
{
    Q_OBJECT
public:
    CommLinkGUI();

private slots:
    void onConnect();
    void onSend();
    void onStartReceive();
    void onStopReceive();
    void onDataReceived(const DataMessage &msg, const QString &source, const QString &timestamp);
    void onLoadMessage();
    void onSaveMessage();
    void onExportLogs();
    void onExportMessages();
    void onClearMessages();
    void onThemeChanged();
    void onToggleLightMode();
    void onToggleDarkMode();
    void onToggleAutoMode();
    void onWsConnected();
    void onWsDisconnected();
    void onWsError(const QString& error);
    void onClientProtocolChanged(int index);
    void onServerProtocolChanged(int index);
    void onSendModeChanged(int index);
    void onFormatChanged(int index);
    void onClientConnected(const QString& clientInfo);
    void onClientDisconnected(const QString& clientInfo);
    void onHttpRequestSent(const QString& method, const QString& url);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void setupValidators();
    void updateStatusBar();
    void saveSettings();
    void logMessage(const QString &message, const QString &prefix = "");
    bool validateInputs();
    void updateClientStatus();
    void updateServerStatus();
    void updateSendButtonState();
    void updateFieldVisibility();

    // UI Components
    QComboBox *protocolCombo;
    QComboBox *dataFormatCombo;
    QComboBox *receiveProtocolCombo;
    QComboBox *httpMethodCombo;
    QLineEdit *hostEdit, *portEdit, *receivePortEdit;
    QPushButton *connectBtn, *sendBtn, *startReceiveBtn, *stopReceiveBtn;
    QPushButton *loadJsonBtn, *saveJsonBtn, *exportLogsBtn, *exportMessagesBtn, *clearMessagesBtn;
    QTextEdit *jsonEdit, *logEdit, *receivedEdit;
    QStatusBar *statusBar;
    QMenuBar *menuBar;
    QAction *lightModeAction;
    QAction *darkModeAction;
    QAction *autoModeAction;
    LoggerWidget *logger;
    
    // Enhanced UI components
    QLabel *clientStatusLabel;
    QLabel *serverStatusLabel;
    QComboBox *sendModeCombo;
    QComboBox *targetClientCombo;
    QListWidget *connectedClientsList;
    QLabel *clientCountLabel;

    // Validators
    QIntValidator *portValidator;

    // Business logic
    TcpClient *tcpClient;
    TcpServer *tcpServer;
    UdpClient *udpClient;
    UdpServer *udpServer;
    WebSocketClient *wsClient;
    WebSocketServer *wsServer;
    HttpClient *httpClient;
    HttpServer *httpServer;
    MessageHistoryManager historyManager;
    QList<DataMessage> receivedMessages;

    // Constants
    static const int DEFAULT_WINDOW_WIDTH = 1000;
    static const int DEFAULT_WINDOW_HEIGHT = 700;
    static const int MIN_WINDOW_WIDTH = 800;
    static const int MIN_WINDOW_HEIGHT = 600;
    static const int ICON_SIZE_16 = 16;
    static const int ICON_SIZE_32 = 32;
    static const int ICON_SIZE_64 = 64;
    static const int ICON_SIZE_128 = 128;
    static const int ICON_SIZE_256 = 256;
    static const int DEFAULT_HTTP_PORT = 80;
    static const int MAX_PORT_NUMBER = 65535;
    static const int MAIN_LAYOUT_SPACING = 8;
};
