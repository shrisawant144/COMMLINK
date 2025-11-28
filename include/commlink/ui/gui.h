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
#include <QtGui/QIntValidator>
#include <QtGui/QCloseEvent>
#include <QtCore/QJsonDocument>
#include <QtCore/QSettings>
#include "../core/dataformat.h"
#include "../network/sender.h"
#include "../network/receiver.h"
#include "../network/websocketclient.h"
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
    void onDataReceived(const DataMessage &msg, const QString &protocol, const QString &senderInfo);
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

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void setupValidators();
    void updateConnectionState(bool connected);
    void updateReceiveState(bool receiving);
    void updateStatusBar();
    void saveSettings();
    void logMessage(const QString &message, const QString &prefix = "");
    bool validateInputs();

    // UI Components
    QComboBox *protocolCombo;
    QComboBox *dataFormatCombo;
    QComboBox *receiveProtocolCombo;
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

    // Validators
    QIntValidator *portValidator;

    // Business logic
    Sender sender;
    Receiver receiver;
    WebSocketClient *wsClient;
    MessageHistoryManager historyManager;
    QList<DataMessage> receivedMessages;
    bool isConnected = false;
    bool isReceiving = false;
};
