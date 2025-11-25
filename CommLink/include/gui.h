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
#include "sender.h"
#include "receiver.h"
#include "filemanager.h"
#include "exportmanager.h"
#include "messagehistorymanager.h"

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
    void onJsonReceived(const QJsonDocument &doc, const QString &protocol, const QString &senderInfo);
    void onLoadJson();
    void onSaveJson();
    void onExportLogs();
    void onExportMessages();
    void onClearMessages();

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
    QComboBox *receiveProtocolCombo;
    QLineEdit *hostEdit, *portEdit, *receiveHostEdit, *receivePortEdit;
    QPushButton *connectBtn, *sendBtn, *startReceiveBtn, *stopReceiveBtn;
    QPushButton *loadJsonBtn, *saveJsonBtn, *exportLogsBtn, *exportMessagesBtn, *clearMessagesBtn;
    QTextEdit *jsonEdit, *logEdit, *receivedEdit;
    QStatusBar *statusBar;
    QMenuBar *menuBar;

    // Validators
    QIntValidator *portValidator;

    // Business logic
    Sender sender;
    Receiver receiver;
    MessageHistoryManager historyManager;
    bool isConnected = false;
    bool isReceiving = false;
};
