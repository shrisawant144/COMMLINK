#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QPushButton>
#include <QtCore/QString>
#include "../core/dataformat.h"
#include "../core/messagehistorymanager.h"

// Forward declaration
class HistoryTab;

/**
 * @brief Panel for displaying sent and received messages
 * 
 * Provides tabbed interface for viewing:
 * - Received messages (client)
 * - Received messages (server)
 * - Sent messages
 * - Combined view
 * - Application logs
 * - Message history
 */
class DisplayPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayPanel(MessageHistoryManager *historyMgr = nullptr, QWidget *parent = nullptr);
    ~DisplayPanel() override = default;

    // Message display operations
    void appendReceivedMessage(const QString &message, bool isServerMessage = false);
    void appendSentMessage(const QString &message);
    void appendLogMessage(const QString &message);

    // Clear operations
    void clearAllMessages();
    void clearReceivedMessages();
    void clearSentMessages();
    void clearLogs();

    // Getters for export
    QString getAllReceivedMessages() const;
    QString getClientReceivedMessages() const;
    QString getServerReceivedMessages() const;
    QString getSentMessages() const;
    QString getLogs() const;

signals:
    void exportMessagesRequested();
    void exportLogsRequested();

private:
    void setupUI();
    void applyStyles();
    void setupAccessibility();

    // UI Components
    QTabWidget *tabWidget;
    QTextEdit *clientReceivedEdit;
    QTextEdit *serverReceivedEdit;
    QTextEdit *sentEdit;
    QTextEdit *allMessagesEdit;
    QTextEdit *logsEdit;
    HistoryTab *historyTab;
    
    QPushButton *clearBtn;
    QPushButton *exportMessagesBtn;
    QPushButton *exportLogsBtn;

    // Business logic
    MessageHistoryManager *historyManager;

    // Constants
    static constexpr int BTN_HEIGHT = 32;
};
