#include "commlink/ui/statuspanel.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>

StatusPanel::StatusPanel(QWidget *parent)
    : QWidget(parent)
    , clientStatusLabel(nullptr)
    , serverStatusLabel(nullptr)
    , clientProtocolLabel(nullptr)
    , serverProtocolLabel(nullptr)
    , clientCountLabel(nullptr)
    , statusBar(nullptr)
{
    setupUI();
    applyStyles();
    setupAccessibility();
}

void StatusPanel::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *group = new QGroupBox("Connection Status");
    auto *gridLayout = new QGridLayout(group);

    // Client status
    auto *clientLabel = new QLabel("Client:");
    clientLabel->setStyleSheet("font-weight: bold;");
    clientStatusLabel = new QLabel("Disconnected");
    clientProtocolLabel = new QLabel("TCP");
    clientProtocolLabel->setStyleSheet("color: #6c757d; font-size: 11px;");

    // Server status
    auto *serverLabel = new QLabel("Server:");
    serverLabel->setStyleSheet("font-weight: bold;");
    serverStatusLabel = new QLabel("Stopped");
    serverProtocolLabel = new QLabel("TCP Server");
    serverProtocolLabel->setStyleSheet("color: #6c757d; font-size: 11px;");

    // Client count
    auto *countLabel = new QLabel("Clients:");
    countLabel->setStyleSheet("font-weight: bold;");
    clientCountLabel = new QLabel("0");

    // Layout
    gridLayout->addWidget(clientLabel, 0, 0);
    gridLayout->addWidget(clientStatusLabel, 0, 1);
    gridLayout->addWidget(clientProtocolLabel, 0, 2);
    
    gridLayout->addWidget(serverLabel, 1, 0);
    gridLayout->addWidget(serverStatusLabel, 1, 1);
    gridLayout->addWidget(serverProtocolLabel, 1, 2);
    
    gridLayout->addWidget(countLabel, 2, 0);
    gridLayout->addWidget(clientCountLabel, 2, 1, 1, 2);

    gridLayout->setColumnStretch(2, 1);

    mainLayout->addWidget(group);

    // Status bar at bottom
    statusBar = new QStatusBar();
    statusBar->setStyleSheet("QStatusBar { border-top: 1px solid #ddd; padding: 4px; }");
    statusBar->showMessage("Ready");
    mainLayout->addWidget(statusBar);
}

void StatusPanel::applyStyles()
{
    setClientStatus("Disconnected", false);
    setServerStatus("Stopped", false);
}

void StatusPanel::setClientStatus(const QString &status, bool isConnected)
{
    clientStatusLabel->setText(status);
    if (isConnected) {
        clientStatusLabel->setStyleSheet("color: #28a745; font-weight: bold;");
    } else {
        clientStatusLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
    }
}

void StatusPanel::setServerStatus(const QString &status, bool isRunning)
{
    serverStatusLabel->setText(status);
    if (isRunning) {
        serverStatusLabel->setStyleSheet("color: #28a745; font-weight: bold;");
    } else {
        serverStatusLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
    }
}

void StatusPanel::setProtocolInfo(const QString &clientProtocol, const QString &serverProtocol)
{
    clientProtocolLabel->setText(clientProtocol);
    serverProtocolLabel->setText(serverProtocol);
}

void StatusPanel::setClientCount(int count)
{
    clientCountLabel->setText(QString::number(count));
    if (count > 0) {
        clientCountLabel->setStyleSheet("color: #28a745; font-weight: bold;");
    } else {
        clientCountLabel->setStyleSheet("color: #6c757d;");
    }
}

void StatusPanel::setStatusMessage(const QString &message)
{
    statusBar->showMessage(message);
}

void StatusPanel::setupAccessibility()
{
    // Client status
    clientStatusLabel->setAccessibleName("Client Connection Status");
    clientStatusLabel->setAccessibleDescription("Shows whether the client is connected or disconnected");
    
    // Server status
    serverStatusLabel->setAccessibleName("Server Status");
    serverStatusLabel->setAccessibleDescription("Shows whether the server is running or stopped");
    
    // Protocol labels
    clientProtocolLabel->setAccessibleName("Client Protocol");
    clientProtocolLabel->setAccessibleDescription("The protocol being used for client connections");
    
    serverProtocolLabel->setAccessibleName("Server Protocol");
    serverProtocolLabel->setAccessibleDescription("The protocol being used for server connections");
    
    // Client count
    clientCountLabel->setAccessibleName("Connected Clients Count");
    clientCountLabel->setAccessibleDescription("Number of clients currently connected to the server");
    
    // Status bar
    statusBar->setAccessibleName("Status Bar");
    statusBar->setAccessibleDescription("Displays status messages and notifications");
}
