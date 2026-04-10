#include "commlink/ui/serverpanel.h"
#include <QtNetwork/QNetworkAddressEntry>
#include <QtNetwork/QNetworkInterface>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtGui/QIntValidator>

ServerPanel::ServerPanel(QWidget *parent)
    : QWidget(parent)
    , protocolCombo(nullptr)
    , bindAddressCombo(nullptr)
    , portEdit(nullptr)
    , startBtn(nullptr)
    , stopBtn(nullptr)
    , clientsList(nullptr)
    , clientCountLabel(nullptr)
    , sendModeCombo(nullptr)
    , targetClientCombo(nullptr)
    , serverRunning(false)
{
    setupUI();
    applyStyles();
    setupAccessibility();
}

void ServerPanel::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *group = new QGroupBox("Server Configuration");
    auto *gridLayout = new QGridLayout(group);

    // Protocol selection
    protocolCombo = new QComboBox();
    protocolCombo->addItems({"TCP Server", "UDP Server", "WebSocket Server", "HTTP Server"});
    protocolCombo->setMinimumHeight(MIN_HEIGHT);
    protocolCombo->setToolTip(
        "TCP Server: Multiple concurrent connections, reliable\n"
        "UDP Server: Connectionless, listens for datagrams\n"
        "WebSocket Server: Persistent bidirectional connections\n"
        "HTTP Server: REST API endpoints with request handling"
    );
    connect(protocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ServerPanel::onProtocolChanged);

    bindAddressCombo = new QComboBox();
    bindAddressCombo->setMinimumHeight(MIN_HEIGHT);
    bindAddressCombo->setToolTip(
        "Select which local address the server should listen on.\n"
        "All Interfaces allows LAN and internet exposure if the network is configured.\n"
        "Loopback Only restricts access to the same machine."
    );
    populateBindAddresses();
    connect(bindAddressCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { emit configurationChanged(); });

    // Port input
    portEdit = new QLineEdit("8080");
    portEdit->setMinimumHeight(MIN_HEIGHT);
    portEdit->setPlaceholderText("Port number");
    portEdit->setValidator(new QIntValidator(1, 65535, this));
    portEdit->setToolTip("Server listening port (1-65535). Avoid privileged ports <1024 unless running as administrator.");
    connect(portEdit, &QLineEdit::textChanged, this, [this](const QString &) {
        emit configurationChanged();
    });

    // Start/Stop buttons
    startBtn = new QPushButton("Start Server");
    startBtn->setMinimumHeight(BTN_HEIGHT);
    connect(startBtn, &QPushButton::clicked, this, &ServerPanel::onStartClicked);

    stopBtn = new QPushButton("Stop Server");
    stopBtn->setMinimumHeight(BTN_HEIGHT);
    stopBtn->setEnabled(false);
    connect(stopBtn, &QPushButton::clicked, this, &ServerPanel::onStopClicked);

    // Connected clients list
    auto *clientsGroup = new QGroupBox("Connected Clients");
    auto *clientsLayout = new QVBoxLayout(clientsGroup);

    clientCountLabel = new QLabel("Clients: 0");
    clientCountLabel->setStyleSheet("font-weight: bold; color: #007bff;");

    clientsList = new QListWidget();
    clientsList->setMaximumHeight(150);
    clientsList->setToolTip("List of currently connected clients");

    clientsLayout->addWidget(clientCountLabel);
    clientsLayout->addWidget(clientsList);
    
    // Send mode controls
    sendModeCombo = new QComboBox();
    sendModeCombo->addItem("Broadcast to All Clients", 1);
    sendModeCombo->addItem("Send to Selected Client", 2);
    sendModeCombo->setMinimumHeight(MIN_HEIGHT);
    sendModeCombo->setToolTip("Select broadcast to all clients or send to specific client");
    connect(sendModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                targetClientCombo->setEnabled(idx == 1); // Enable when "Send to Selected" is chosen
                emit sendModeChanged(sendModeCombo->currentText());
            });
    
    targetClientCombo = new QComboBox();
    targetClientCombo->setMinimumHeight(MIN_HEIGHT);
    targetClientCombo->setEnabled(false);
    targetClientCombo->setToolTip("Select specific client to send message to (Send to Selected mode only)");
    
    auto *sendModeLayout = new QHBoxLayout();
    sendModeLayout->addWidget(new QLabel("Send Mode:"));
    sendModeLayout->addWidget(sendModeCombo, 1);
    
    auto *targetLayout = new QHBoxLayout();
    targetLayout->addWidget(new QLabel("Target Client:"));
    targetLayout->addWidget(targetClientCombo, 1);
    
    clientsLayout->addLayout(sendModeLayout);
    clientsLayout->addLayout(targetLayout);

    // Layout
    gridLayout->addWidget(new QLabel("Protocol:"), 0, 0);
    gridLayout->addWidget(protocolCombo, 0, 1);
    gridLayout->addWidget(new QLabel("Bind Address:"), 1, 0);
    gridLayout->addWidget(bindAddressCombo, 1, 1);
    gridLayout->addWidget(new QLabel("Port:"), 2, 0);
    gridLayout->addWidget(portEdit, 2, 1);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(startBtn);
    btnLayout->addWidget(stopBtn);
    gridLayout->addLayout(btnLayout, 3, 0, 1, 2);

    mainLayout->addWidget(group);
    mainLayout->addWidget(clientsGroup);
}

void ServerPanel::applyStyles()
{
    startBtn->setStyleSheet(
        "QPushButton { "
        "font-weight: bold; "
        "background-color: #28a745; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px; "
        "}"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:pressed { background-color: #1e7e34; }"
        "QPushButton:disabled { background-color: #6c757d; }"
    );

    stopBtn->setStyleSheet(
        "QPushButton { "
        "font-weight: bold; "
        "background-color: #dc3545; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px; "
        "}"
        "QPushButton:hover { background-color: #c82333; }"
        "QPushButton:pressed { background-color: #bd2130; }"
        "QPushButton:disabled { background-color: #6c757d; }"
    );
}

void ServerPanel::onStartClicked()
{
    emit startServerRequested();
}

void ServerPanel::onStopClicked()
{
    emit stopServerRequested();
}

void ServerPanel::onProtocolChanged(int index)
{
    Q_UNUSED(index);
    emit protocolChanged(getProtocol());
    emit configurationChanged();
}

void ServerPanel::updateClientCount()
{
    int count = clientsList->count();
    clientCountLabel->setText(QString("Clients: %1").arg(count));
    clientCountLabel->setStyleSheet(count > 0 ? 
        "font-weight: bold; color: #28a745;" : 
        "font-weight: bold; color: #6c757d;");
}

// Getters
QString ServerPanel::getProtocol() const
{
    return protocolCombo->currentText();
}

int ServerPanel::getPort() const
{
    return portEdit->text().toInt();
}

QHostAddress ServerPanel::getBindAddress() const
{
    return QHostAddress(bindAddressCombo->currentData().toString());
}

QString ServerPanel::getBindAddressText() const
{
    return bindAddressCombo->currentData().toString();
}

bool ServerPanel::isServerRunning() const
{
    return serverRunning;
}

// Setters
void ServerPanel::setServerState(bool running)
{
    serverRunning = running;
    startBtn->setEnabled(!running);
    stopBtn->setEnabled(running);
    protocolCombo->setEnabled(!running);
    bindAddressCombo->setEnabled(!running);
    portEdit->setEnabled(!running);
}

void ServerPanel::setProtocol(const QString &protocol)
{
    int index = protocolCombo->findText(protocol);
    if (index >= 0) {
        protocolCombo->setCurrentIndex(index);
    }
}

void ServerPanel::setPort(int port)
{
    portEdit->setText(QString::number(port));
}

void ServerPanel::setBindAddress(const QString &bindAddress)
{
    int index = bindAddressCombo->findData(bindAddress);
    if (index >= 0) {
        bindAddressCombo->setCurrentIndex(index);
    }
}

// Client management
void ServerPanel::addClient(const QString &clientInfo)
{
    clientsList->addItem(clientInfo);
    targetClientCombo->addItem(clientInfo);
    updateClientCount();
}

void ServerPanel::removeClient(const QString &clientInfo)
{
    auto items = clientsList->findItems(clientInfo, Qt::MatchExactly);
    for (auto *item : items) {
        delete clientsList->takeItem(clientsList->row(item));
    }
    
    // Remove from target combo
    int index = targetClientCombo->findText(clientInfo);
    if (index >= 0) {
        targetClientCombo->removeItem(index);
    }
    
    updateClientCount();
}

void ServerPanel::clearClients()
{
    clientsList->clear();
    targetClientCombo->clear();
    updateClientCount();
}

int ServerPanel::getClientCount() const
{
    return clientsList->count();
}

QString ServerPanel::getClientAt(int index) const
{
    if (index >= 0 && index < clientsList->count()) {
        return clientsList->item(index)->text();
    }
    return QString();
}

QString ServerPanel::getSendMode() const
{
    return sendModeCombo->currentText();
}

QString ServerPanel::getTargetClient() const
{
    return targetClientCombo->currentText();
}

void ServerPanel::setupAccessibility()
{
    // Protocol combo
    protocolCombo->setAccessibleName("Server Protocol Selection");
    protocolCombo->setAccessibleDescription("Select the protocol for the server: TCP, UDP, or WebSocket");

    bindAddressCombo->setAccessibleName("Server Bind Address");
    bindAddressCombo->setAccessibleDescription("Select the local address the server will listen on");
    
    // Port edit
    portEdit->setAccessibleName("Server Port");
    portEdit->setAccessibleDescription("Enter the port number for the server (1-65535)");
    
    // Start/Stop buttons
    startBtn->setAccessibleName("Start Server");
    startBtn->setAccessibleDescription("Start the server with the selected protocol and port");
    
    stopBtn->setAccessibleName("Stop Server");
    stopBtn->setAccessibleDescription("Stop the running server");
    
    // Clients list
    clientsList->setAccessibleName("Connected Clients List");
    clientsList->setAccessibleDescription("List of currently connected clients to this server");
    
    // Client count label
    clientCountLabel->setAccessibleName("Client Count");
    clientCountLabel->setAccessibleDescription("Number of clients currently connected to the server");
    
    // Send mode controls
    sendModeCombo->setAccessibleName("Server Send Mode");
    sendModeCombo->setAccessibleDescription("Select broadcast to all clients or send to specific client");
    
    targetClientCombo->setAccessibleName("Target Client Selection");
    targetClientCombo->setAccessibleDescription("Select the specific client to send the message to");
}

void ServerPanel::populateBindAddresses()
{
    bindAddressCombo->clear();
    bindAddressCombo->addItem("All Interfaces (0.0.0.0)", "0.0.0.0");
    bindAddressCombo->addItem("Loopback Only (127.0.0.1)", "127.0.0.1");

    QStringList seenAddresses;
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            !(iface.flags() & QNetworkInterface::IsRunning)) {
            continue;
        }

        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            const QHostAddress ip = entry.ip();
            if (ip.protocol() != QAbstractSocket::IPv4Protocol) {
                continue;
            }

            const QString ipString = ip.toString();
            if (ipString.isEmpty() || ipString == "0.0.0.0" || seenAddresses.contains(ipString)) {
                continue;
            }

            seenAddresses.append(ipString);
            bindAddressCombo->addItem(QString("%1 (%2)").arg(iface.humanReadableName(), ipString), ipString);
        }
    }
}
