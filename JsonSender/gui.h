#pragma once
#include <QWidget>
#include "sender.h"

class QComboBox;
class QLineEdit;
class QPushButton;
class QTextEdit;

class JsonSenderGUI : public QWidget
{
    Q_OBJECT
public:
    JsonSenderGUI();
    
private slots:
    void onConnect();
    void onSend();
    
private:
    QComboBox *protocolCombo;
    QLineEdit *hostEdit, *portEdit;
    QPushButton *connectBtn, *sendBtn;
    QTextEdit *jsonEdit, *logEdit;
    Sender sender;
};