#include "commlink/ui/connectionpanel.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtGui/QIntValidator>

ConnectionPanel::ConnectionPanel(QWidget *parent)
    : QWidget(parent)
    , protocolCombo(nullptr)
    , httpMethodCombo(nullptr)
    , hostEdit(nullptr)
    , portEdit(nullptr)
    , connectBtn(nullptr)
    , httpPollingCheckbox(nullptr)
    , infoLabel(nullptr)
    , connected(false)
{
    setupUI();
    applyStyles();
    setupAccessibility();
}

void ConnectionPanel::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *group = new QGroupBox("Client Configuration");
    auto *gridLayout = new QGridLayout(group);

    // Protocol selection
    protocolCombo = new QComboBox();
    protocolCombo->addItems({"TCP", "UDP", "WebSocket", "HTTP"});
    protocolCombo->setMinimumHeight(MIN_HEIGHT);
    protocolCombo->setToolTip(
        "TCP: Reliable, connection-oriented, guaranteed delivery\n"
        "UDP: Fast, connectionless, no delivery guarantee\n"
        "WebSocket: Full-duplex, persistent connection over HTTP\n"
        "HTTP: Request-response, stateless, widely compatible"
    );
    connect(protocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConnectionPanel::onProtocolChanged);

    // HTTP method selection
    httpMethodCombo = new QComboBox();
    httpMethodCombo->addItems({"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"});
    httpMethodCombo->setCurrentText("POST");
    httpMethodCombo->setMinimumHeight(MIN_HEIGHT);
    httpMethodCombo->setVisible(false);
    httpMethodCombo->setToolTip(
        "GET: Retrieve data\n"
        "POST: Submit data\n"
        "PUT: Update resource\n"
        "DELETE: Remove resource\n"
        "PATCH: Partial update\n"
        "HEAD: Get headers only\n"
        "OPTIONS: Get supported methods"
    );

    // HTTP polling option
    httpPollingCheckbox = new QCheckBox("Enable Long-Polling (Auto-receive messages)");
    httpPollingCheckbox->setVisible(false);
    httpPollingCheckbox->setToolTip(
        "Enable automatic polling to receive messages from HTTP server.\n"
        "Polls every 2 seconds. Useful for simulating real-time updates over HTTP."
    );
    connect(httpPollingCheckbox, &QCheckBox::toggled,
            this, &ConnectionPanel::httpPollingToggled);

    // Info label
    infoLabel = new QLabel("TCP/UDP: Host + Port | WebSocket: ws://host:port | HTTP: http://host:port/path");
    infoLabel->setStyleSheet("color: #6c757d; font-size: 10px; font-style: italic;");
    infoLabel->setWordWrap(true);

    // Host input
    hostEdit = new QLineEdit("127.0.0.1");
    hostEdit->setMinimumHeight(MIN_HEIGHT);
    hostEdit->setPlaceholderText("Host/IP, ws://host:port, or http://host:port/path");
    hostEdit->setToolTip("Enter host/IP address (TCP/UDP) or full URL (WebSocket: ws://host:port, HTTP: http://host:port/path)");

    // Port input
    portEdit = new QLineEdit("5000");
    portEdit->setMinimumHeight(MIN_HEIGHT);
    portEdit->setPlaceholderText("Port number");
    portEdit->setValidator(new QIntValidator(1, 65535, this));
    portEdit->setToolTip("Port number (1-65535). Avoid privileged ports <1024 unless running with elevated permissions.");

    // Connect button
    connectBtn = new QPushButton("Connect");
    connectBtn->setMinimumHeight(BTN_HEIGHT);
    connect(connectBtn, &QPushButton::clicked, this, &ConnectionPanel::onConnectClicked);

    // Layout
    gridLayout->addWidget(new QLabel("Protocol:"), 0, 0);
    gridLayout->addWidget(protocolCombo, 0, 1);
    gridLayout->addWidget(new QLabel("HTTP Method:"), 1, 0);
    gridLayout->addWidget(httpMethodCombo, 1, 1);
    gridLayout->addWidget(httpPollingCheckbox, 2, 0, 1, 2);
    gridLayout->addWidget(infoLabel, 3, 0, 1, 2);
    gridLayout->addWidget(new QLabel("Host:"), 4, 0);
    gridLayout->addWidget(hostEdit, 4, 1);
    gridLayout->addWidget(new QLabel("Port:"), 5, 0);
    gridLayout->addWidget(portEdit, 5, 1);
    gridLayout->addWidget(connectBtn, 6, 0, 1, 2);

    mainLayout->addWidget(group);
}

void ConnectionPanel::applyStyles()
{
    connectBtn->setStyleSheet(
        "QPushButton { "
        "font-weight: bold; "
        "background-color: #007bff; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px; "
        "}"
        "QPushButton:hover { background-color: #0056b3; }"
        "QPushButton:pressed { background-color: #004085; }"
        "QPushButton:disabled { background-color: #6c757d; }"
    );
}

void ConnectionPanel::onProtocolChanged(int index)
{
    QString protocol = protocolCombo->itemText(index);
    updateFieldVisibility();
    emit protocolChanged(protocol);
}

void ConnectionPanel::updateFieldVisibility()
{
    QString protocol = getProtocol();
    bool isHttp = (protocol == "HTTP");
    bool isWebSocket = (protocol == "WebSocket");

    httpMethodCombo->setVisible(isHttp);
    httpPollingCheckbox->setVisible(isHttp);
    
    // Update port visibility based on protocol
    bool showPort = !(isWebSocket || isHttp);
    portEdit->setVisible(showPort);
    
    // Find and update port label visibility
    auto labels = findChildren<QLabel*>();
    for (auto *label : labels) {
        if (label->text() == "Port:") {
            label->setVisible(showPort);
            break;
        }
    }
    
    // Update placeholder text based on protocol
    if (isHttp) {
        hostEdit->setPlaceholderText("http://host:port/path or https://host:port/path");
    } else if (isWebSocket) {
        hostEdit->setPlaceholderText("ws://host:port or wss://host:port");
    } else {
        hostEdit->setPlaceholderText("Host/IP address");
    }
}

void ConnectionPanel::onConnectClicked()
{
    if (connected) {
        emit disconnectRequested();
    } else {
        emit connectRequested();
    }
}

// Getters
QString ConnectionPanel::getProtocol() const
{
    return protocolCombo->currentText();
}

QString ConnectionPanel::getHost() const
{
    return hostEdit->text().trimmed();
}

int ConnectionPanel::getPort() const
{
    return portEdit->text().toInt();
}

QString ConnectionPanel::getHttpMethod() const
{
    return httpMethodCombo->currentText();
}

bool ConnectionPanel::isHttpPollingEnabled() const
{
    return httpPollingCheckbox->isChecked();
}

bool ConnectionPanel::isConnected() const
{
    return connected;
}

// Setters
void ConnectionPanel::setConnectionState(bool isConnected)
{
    connected = isConnected;
    connectBtn->setText(connected ? "Disconnect" : "Connect");
    connectBtn->setStyleSheet(connected ?
        "QPushButton { font-weight: bold; background-color: #dc3545; color: white; border: none; border-radius: 4px; padding: 8px; }"
        "QPushButton:hover { background-color: #c82333; }"
        :
        "QPushButton { font-weight: bold; background-color: #007bff; color: white; border: none; border-radius: 4px; padding: 8px; }"
        "QPushButton:hover { background-color: #0056b3; }"
    );
    
    // Disable configuration when connected
    protocolCombo->setEnabled(!connected);
    httpMethodCombo->setEnabled(!connected);
    hostEdit->setEnabled(!connected);
    portEdit->setEnabled(!connected);
}

void ConnectionPanel::setProtocol(const QString &protocol)
{
    int index = protocolCombo->findText(protocol);
    if (index >= 0) {
        protocolCombo->setCurrentIndex(index);
    }
}

void ConnectionPanel::setHost(const QString &host)
{
    hostEdit->setText(host);
}

void ConnectionPanel::setPort(int port)
{
    portEdit->setText(QString::number(port));
}

void ConnectionPanel::setupAccessibility()
{
    // Set accessible properties for the panel
    setAccessibleName("Connection Configuration Panel");
    setAccessibleDescription("Configure client connection settings including protocol, host, and port");
    
    // Set accessible properties for controls
    protocolCombo->setAccessibleName("Client Protocol Selector");
    protocolCombo->setAccessibleDescription("Select network protocol: TCP, UDP, WebSocket, or HTTP");
    
    httpMethodCombo->setAccessibleName("HTTP Method Selector");
    httpMethodCombo->setAccessibleDescription("Select HTTP request method");
    
    hostEdit->setAccessibleName("Host Address Input");
    hostEdit->setAccessibleDescription("Enter server host address or URL");
    
    portEdit->setAccessibleName("Port Number Input");
    portEdit->setAccessibleDescription("Enter server port number (1-65535)");
    
    connectBtn->setAccessibleName("Connect Button");
    connectBtn->setAccessibleDescription("Connect to or disconnect from server");
    
    httpPollingCheckbox->setAccessibleName("HTTP Long-Polling Checkbox");
    httpPollingCheckbox->setAccessibleDescription("Enable automatic message polling for HTTP connections");
}
