#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QTableWidgetItem>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include "messagehistorymanager.h"

class HistoryTab : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryTab(MessageHistoryManager *historyManager, QWidget *parent = nullptr);
    ~HistoryTab();

private slots:
    void refreshMessages();
    void onSearchTextChanged();
    void onFilterChanged();
    void onExportClicked();
    void onClearHistoryClicked();
    void onTableItemDoubleClicked(QTableWidgetItem *item);
    void smartRefresh();

private:
    void setupUI();
    void populateTable();
    void updateTable();
    void showMessageDetails(const QVariantMap &message);

    MessageHistoryManager *historyManager;

    // UI Components
    QLineEdit *searchEdit;
    QComboBox *directionCombo;
    QComboBox *protocolCombo;
    QDateTimeEdit *startDateEdit;
    QDateTimeEdit *endDateEdit;
    QPushButton *refreshBtn;
    QPushButton *exportBtn;
    QPushButton *clearBtn;
    QTableWidget *tableWidget;
    QLabel *statusLabel;

    // Data
    QList<QVariantMap> currentMessages;
    QTimer *refreshTimer;
    QDateTime lastRefreshTime;
    int currentMessageCount;
};
