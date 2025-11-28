#include "commlink/historytab.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QTextEdit>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>

HistoryTab::HistoryTab(MessageHistoryManager *historyManager, QWidget *parent)
    : QWidget(parent), historyManager(historyManager), currentMessageCount(0)
{
    setupUI();
    lastRefreshTime = QDateTime::currentDateTime();
    refreshMessages();
}

HistoryTab::~HistoryTab()
{
    if (refreshTimer) {
        refreshTimer->stop();
        delete refreshTimer;
    }
}

void HistoryTab::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    // Search and filter controls
    auto *controlsLayout = new QHBoxLayout();

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search messages...");
    controlsLayout->addWidget(new QLabel("Search:"));
    controlsLayout->addWidget(searchEdit);

    directionCombo = new QComboBox();
    directionCombo->addItem("All Directions", "");
    directionCombo->addItem("Sent", "sent");
    directionCombo->addItem("Received", "received");
    controlsLayout->addWidget(new QLabel("Direction:"));
    controlsLayout->addWidget(directionCombo);

    protocolCombo = new QComboBox();
    protocolCombo->addItem("All Protocols", "");
    protocolCombo->addItem("TCP", "TCP");
    protocolCombo->addItem("UDP", "UDP");
    controlsLayout->addWidget(new QLabel("Protocol:"));
    controlsLayout->addWidget(protocolCombo);

    refreshBtn = new QPushButton("🔄 Refresh");
    controlsLayout->addWidget(refreshBtn);

    mainLayout->addLayout(controlsLayout);

    // Date filters
    auto *dateLayout = new QHBoxLayout();
    startDateEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-7));
    startDateEdit->setCalendarPopup(true);
    endDateEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    endDateEdit->setCalendarPopup(true);

    dateLayout->addWidget(new QLabel("From:"));
    dateLayout->addWidget(startDateEdit);
    dateLayout->addWidget(new QLabel("To:"));
    dateLayout->addWidget(endDateEdit);
    dateLayout->addStretch();

    mainLayout->addLayout(dateLayout);

    // Table
    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(7);
    tableWidget->setHorizontalHeaderLabels({"ID", "Timestamp", "Direction", "Protocol", "Host", "Port", "Content"});
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(tableWidget);

    // Action buttons
    auto *buttonLayout = new QHBoxLayout();
    exportBtn = new QPushButton("📤 Export");
    clearBtn = new QPushButton("🗑️ Clear History");
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addWidget(clearBtn);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(searchEdit, &QLineEdit::textChanged, this, &HistoryTab::onSearchTextChanged);
    connect(directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistoryTab::onFilterChanged);
    connect(protocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistoryTab::onFilterChanged);
    connect(startDateEdit, &QDateTimeEdit::dateTimeChanged, this, &HistoryTab::onFilterChanged);
    connect(endDateEdit, &QDateTimeEdit::dateTimeChanged, this, &HistoryTab::onFilterChanged);
    connect(refreshBtn, &QPushButton::clicked, this, &HistoryTab::refreshMessages);
    connect(exportBtn, &QPushButton::clicked, this, &HistoryTab::onExportClicked);
    connect(clearBtn, &QPushButton::clicked, this, &HistoryTab::onClearHistoryClicked);

    // Status label
    statusLabel = new QLabel("Ready");
    mainLayout->addWidget(statusLabel);

    // Auto refresh timer with smart refresh
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &HistoryTab::smartRefresh);
    refreshTimer->start(3000); // Check every 3 seconds
    
    // Double-click to view details
    connect(tableWidget, &QTableWidget::itemDoubleClicked, this, &HistoryTab::onTableItemDoubleClicked);
}

void HistoryTab::refreshMessages()
{
    updateTable();
}

void HistoryTab::onSearchTextChanged()
{
    updateTable();
}

void HistoryTab::onFilterChanged()
{
    updateTable();
}

void HistoryTab::updateTable()
{
    QString filter = searchEdit->text();
    QString direction = directionCombo->currentData().toString();
    QString protocol = protocolCombo->currentData().toString();
    QDateTime startDate = startDateEdit->dateTime();
    QDateTime endDate = endDateEdit->dateTime();

    // Build filter string
    QString combinedFilter;
    if (!direction.isEmpty()) {
        combinedFilter += QString("direction:%1 ").arg(direction);
    }
    if (!protocol.isEmpty()) {
        combinedFilter += QString("protocol:%1 ").arg(protocol);
    }
    combinedFilter += filter;

    currentMessages = historyManager->getMessages(combinedFilter.trimmed(), startDate, endDate, 1000, 0);
    currentMessageCount = currentMessages.size();

    tableWidget->setRowCount(currentMessages.size());
    tableWidget->setSortingEnabled(false); // Disable during update for performance
    
    for (int i = 0; i < currentMessages.size(); ++i) {
        const QVariantMap &msg = currentMessages[i];
        tableWidget->setItem(i, 0, new QTableWidgetItem(msg["id"].toString()));
        tableWidget->setItem(i, 1, new QTableWidgetItem(msg["timestamp"].toString()));
        
        // Color code direction
        auto *directionItem = new QTableWidgetItem(msg["direction"].toString());
        if (msg["direction"].toString() == "sent") {
            directionItem->setBackground(QColor(200, 255, 200)); // Light green
        } else {
            directionItem->setBackground(QColor(200, 200, 255)); // Light blue
        }
        tableWidget->setItem(i, 2, directionItem);
        
        tableWidget->setItem(i, 3, new QTableWidgetItem(msg["protocol"].toString()));
        tableWidget->setItem(i, 4, new QTableWidgetItem(msg["host"].toString()));
        tableWidget->setItem(i, 5, new QTableWidgetItem(msg["port"].toString()));
        
        // Truncate content for display
        QString content = msg["content"].toString();
        if (content.length() > 100) {
            content = content.left(97) + "...";
        }
        tableWidget->setItem(i, 6, new QTableWidgetItem(content));
    }
    
    tableWidget->setSortingEnabled(true);
    statusLabel->setText(QString("Showing %1 messages").arg(currentMessages.size()));
}

void HistoryTab::onExportClicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Export History", QString(), "JSON Files (*.json);;CSV Files (*.csv);;All Files (*)");
    if (!filePath.isEmpty()) {
        QString format = "json";
        if (filePath.endsWith(".csv")) format = "csv";
        if (historyManager->exportMessages(filePath, format)) {
            QMessageBox::information(this, "Success", "History exported successfully!");
        } else {
            QMessageBox::warning(this, "Error", "Failed to export history.");
        }
    }
}

void HistoryTab::onClearHistoryClicked()
{
    if (QMessageBox::question(this, "Clear History", "Are you sure you want to clear all message history? This cannot be undone.") == QMessageBox::Yes) {
        if (historyManager->clearAllMessages()) {
            QMessageBox::information(this, "Success", "History cleared successfully!");
            refreshMessages();
        } else {
            QMessageBox::warning(this, "Error", "Failed to clear history.");
        }
    }
}

void HistoryTab::onTableItemDoubleClicked(QTableWidgetItem *item)
{
    if (!item) return;
    
    int row = item->row();
    if (row >= 0 && row < currentMessages.size()) {
        showMessageDetails(currentMessages[row]);
    }
}

void HistoryTab::smartRefresh()
{
    // Only refresh if data has changed
    QDateTime lastUpdate = historyManager->getLastMessageTime();
    if (lastUpdate > lastRefreshTime) {
        refreshMessages();
        lastRefreshTime = QDateTime::currentDateTime();
    }
}

void HistoryTab::showMessageDetails(const QVariantMap &message)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Message Details");
    dialog.resize(600, 400);
    
    auto *layout = new QVBoxLayout(&dialog);
    
    auto *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    
    QString details = QString(
        "ID: %1\n"
        "Timestamp: %2\n"
        "Direction: %3\n"
        "Protocol: %4\n"
        "Host: %5\n"
        "Port: %6\n"
        "Sender Info: %7\n"
        "Session ID: %8\n\n"
        "Content:\n%9"
    ).arg(
        message["id"].toString(),
        message["timestamp"].toString(),
        message["direction"].toString(),
        message["protocol"].toString(),
        message["host"].toString(),
        message["port"].toString(),
        message["sender_info"].toString(),
        message["session_id"].toString(),
        message["content"].toString()
    );
    
    textEdit->setPlainText(details);
    layout->addWidget(textEdit);
    
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttonBox);
    
    dialog.exec();
}
