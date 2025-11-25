#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>
#include <QtGui/QIntValidator>
#include <QtCore/QJsonDocument>
#include <QtCore/QSettings>
#include "sender.h"
#include "receiver.h"

class JsonSenderGUI : public QWidget
{
    Q_OBJECT
public:
    JsonSenderGUI();

private slots:
    void onConnect();
    void onSend();
    void onStartReceive();
    void onStopReceive();
    void onJsonReceived(const QJsonDocument &doc, const QString &protocol, const QString &senderInfo);

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
    QTextEdit *jsonEdit, *logEdit, *receivedEdit;
    QStatusBar *statusBar;

    // Validators
    QIntValidator *portValidator;

    // Business logic
    Sender sender;
    Receiver receiver;
    bool isConnected = false;
    bool isReceiving = false;
};
