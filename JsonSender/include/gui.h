#pragma once
#include <QWidget>
#include <QJsonDocument>
#include "sender.h"

class QComboBox;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QIntValidator;

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
    void logMessage(const QString &message, const QString &prefix = "");
    bool validateInputs();
    
    // UI Components
    QComboBox *protocolCombo;
    QLineEdit *hostEdit, *portEdit, *receivePortEdit;
    QPushButton *connectBtn, *sendBtn, *startReceiveBtn, *stopReceiveBtn;
    QTextEdit *jsonEdit, *logEdit, *receivedEdit;
    
    // Validators
    QIntValidator *portValidator;
    
    // Business logic
    Sender sender;
    bool isConnected = false;
    bool isReceiving = false;
};