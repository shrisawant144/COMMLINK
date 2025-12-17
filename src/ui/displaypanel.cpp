#include "commlink/ui/displaypanel.h"
#include "commlink/ui/historytab.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtCore/QDateTime>

DisplayPanel::DisplayPanel(MessageHistoryManager *historyMgr, QWidget *parent)
    : QWidget(parent)
    , tabWidget(nullptr)
    , clientReceivedEdit(nullptr)
    , serverReceivedEdit(nullptr)
    , sentEdit(nullptr)
    , allMessagesEdit(nullptr)
    , logsEdit(nullptr)
    , historyTab(nullptr)
    , clearBtn(nullptr)
    , exportMessagesBtn(nullptr)
    , exportLogsBtn(nullptr)
    , historyManager(historyMgr)
{
    setupUI();
    applyStyles();
    setupAccessibility();
}

void DisplayPanel::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *group = new QGroupBox("Messages & Logs");
    auto *layout = new QVBoxLayout(group);

    // Tab widget for different views
    tabWidget = new QTabWidget();
    
    // Client received messages tab
    clientReceivedEdit = new QTextEdit();
    clientReceivedEdit->setReadOnly(true);
    clientReceivedEdit->setPlaceholderText("Messages received by client will appear here...");
    clientReceivedEdit->setToolTip("Display area for messages received by client connections");
    tabWidget->addTab(clientReceivedEdit, "Client Received");

    // Server received messages tab
    serverReceivedEdit = new QTextEdit();
    serverReceivedEdit->setReadOnly(true);
    serverReceivedEdit->setPlaceholderText("Messages received by server will appear here...");
    serverReceivedEdit->setToolTip("Display area for messages received by server connections");
    tabWidget->addTab(serverReceivedEdit, "Server Received");

    // Sent messages tab
    sentEdit = new QTextEdit();
    sentEdit->setReadOnly(true);
    sentEdit->setPlaceholderText("Sent messages will appear here...");
    sentEdit->setToolTip("Display area for messages you have sent");
    tabWidget->addTab(sentEdit, "Sent");

    // Combined messages tab
    allMessagesEdit = new QTextEdit();
    allMessagesEdit->setReadOnly(true);
    allMessagesEdit->setPlaceholderText("All messages will appear here...");
    allMessagesEdit->setToolTip("Combined display of all sent and received messages");
    tabWidget->addTab(allMessagesEdit, "All Messages");

    // Logs tab
    logsEdit = new QTextEdit();
    logsEdit->setReadOnly(true);
    logsEdit->setPlaceholderText("Application logs will appear here...");
    logsEdit->setToolTip("Application logs and system messages");
    tabWidget->addTab(logsEdit, "Logs");

    // History tab (if history manager is available)
    if (historyManager) {
        historyTab = new HistoryTab(historyManager);
        tabWidget->addTab(historyTab, "History");
    }

    layout->addWidget(tabWidget);

    // Control buttons
    auto *btnLayout = new QHBoxLayout();

    clearBtn = new QPushButton("Clear Messages");
    clearBtn->setMinimumHeight(BTN_HEIGHT);
    clearBtn->setToolTip("Clear all message displays (Ctrl+L)");
    connect(clearBtn, &QPushButton::clicked, this, &DisplayPanel::clearAllMessages);

    exportMessagesBtn = new QPushButton("Export Messages");
    exportMessagesBtn->setMinimumHeight(BTN_HEIGHT);
    exportMessagesBtn->setToolTip("Export received messages to file (Ctrl+E)");
    connect(exportMessagesBtn, &QPushButton::clicked, this, &DisplayPanel::exportMessagesRequested);

    exportLogsBtn = new QPushButton("Export Logs");
    exportLogsBtn->setMinimumHeight(BTN_HEIGHT);
    exportLogsBtn->setToolTip("Export application logs to file (Ctrl+Shift+E)");
    connect(exportLogsBtn, &QPushButton::clicked, this, &DisplayPanel::exportLogsRequested);

    btnLayout->addWidget(clearBtn);
    btnLayout->addWidget(exportMessagesBtn);
    btnLayout->addWidget(exportLogsBtn);
    btnLayout->addStretch();

    layout->addLayout(btnLayout);

    mainLayout->addWidget(group);
}

void DisplayPanel::applyStyles()
{
    clearBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #ffc107; "
        "color: #212529; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #e0a800; }"
    );

    exportMessagesBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #17a2b8; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #138496; }"
    );

    exportLogsBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #17a2b8; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #138496; }"
    );
}

// Message display operations
void DisplayPanel::appendReceivedMessage(const QString &message, bool isServerMessage)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString formattedMsg = QString("[%1] %2\n").arg(timestamp, message);

    if (isServerMessage) {
        serverReceivedEdit->append(formattedMsg);
    } else {
        clientReceivedEdit->append(formattedMsg);
    }
    
    allMessagesEdit->append(QString("[RCV] %1").arg(formattedMsg));
}

void DisplayPanel::appendSentMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString formattedMsg = QString("[%1] %2\n").arg(timestamp, message);
    
    sentEdit->append(formattedMsg);
    allMessagesEdit->append(QString("[SENT] %1").arg(formattedMsg));
}

void DisplayPanel::appendLogMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString formattedMsg = QString("[%1] %2\n").arg(timestamp, message);
    
    logsEdit->append(formattedMsg);
}

// Clear operations
void DisplayPanel::clearAllMessages()
{
    clientReceivedEdit->clear();
    serverReceivedEdit->clear();
    sentEdit->clear();
    allMessagesEdit->clear();
    appendLogMessage("All messages cleared");
}

void DisplayPanel::clearReceivedMessages()
{
    clientReceivedEdit->clear();
    serverReceivedEdit->clear();
}

void DisplayPanel::clearSentMessages()
{
    sentEdit->clear();
}

void DisplayPanel::clearLogs()
{
    logsEdit->clear();
}

// Getters for export
QString DisplayPanel::getAllReceivedMessages() const
{
    return clientReceivedEdit->toPlainText() + "\n" + serverReceivedEdit->toPlainText();
}

QString DisplayPanel::getClientReceivedMessages() const
{
    return clientReceivedEdit->toPlainText();
}

QString DisplayPanel::getServerReceivedMessages() const
{
    return serverReceivedEdit->toPlainText();
}

QString DisplayPanel::getSentMessages() const
{
    return sentEdit->toPlainText();
}

QString DisplayPanel::getLogs() const
{
    return logsEdit->toPlainText();
}

void DisplayPanel::setupAccessibility()
{
    // Tab widget
    tabWidget->setAccessibleName("Message Display Tabs");
    tabWidget->setAccessibleDescription("Tabbed display showing different message categories: Client Received, Server Received, Sent, All Messages, History, and Logs");
    
    // Client received tab
    clientReceivedEdit->setAccessibleName("Client Received Messages");
    clientReceivedEdit->setAccessibleDescription("Display area for messages received by client connections");
    
    // Server received tab
    serverReceivedEdit->setAccessibleName("Server Received Messages");
    serverReceivedEdit->setAccessibleDescription("Display area for messages received by server connections");
    
    // Sent tab
    sentEdit->setAccessibleName("Sent Messages");
    sentEdit->setAccessibleDescription("Display area for messages that have been sent");
    
    // All messages tab
    allMessagesEdit->setAccessibleName("All Messages");
    allMessagesEdit->setAccessibleDescription("Display area showing all messages from all categories");
    
    // Logs tab
    logsEdit->setAccessibleName("Application Logs");
    logsEdit->setAccessibleDescription("Display area for application logs and diagnostic information");
    
    // History tab (if present)
    if (historyTab) {
        historyTab->setAccessibleName("Message History");
        historyTab->setAccessibleDescription("Searchable history of all messages stored in the database");
    }
    
    // Buttons
    clearBtn->setAccessibleName("Clear Displays");
    clearBtn->setAccessibleDescription("Clear all message display areas");
    
    exportMessagesBtn->setAccessibleName("Export Messages");
    exportMessagesBtn->setAccessibleDescription("Export displayed messages to a file");
    
    exportLogsBtn->setAccessibleName("Export Logs");
    exportLogsBtn->setAccessibleDescription("Export application logs to a file");
}
