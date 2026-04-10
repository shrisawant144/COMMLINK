#include "commlink/ui/networkinfopanel.h"

#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkAddressEntry>
#include <QtNetwork/QNetworkInterface>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

namespace {
QString formatProtocolName(const QString &protocol)
{
    if (protocol == "TCP Server") {
        return "TCP";
    }
    if (protocol == "UDP Server") {
        return "UDP";
    }
    if (protocol == "WebSocket Server") {
        return "WebSocket";
    }
    if (protocol == "HTTP Server") {
        return "HTTP";
    }
    return protocol;
}
}

NetworkInfoPanel::NetworkInfoPanel(QWidget *parent)
    : QWidget(parent)
    , infoEdit(nullptr)
    , refreshBtn(nullptr)
    , currentProtocol("TCP Server")
    , currentBindAddress("0.0.0.0")
    , currentPort(8080)
{
    setupUI();
    applyStyles();
    setupAccessibility();
    refreshNetworkInfo();
}

void NetworkInfoPanel::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *group = new QGroupBox("Network Info");
    auto *layout = new QVBoxLayout(group);

    infoEdit = new QTextEdit();
    infoEdit->setReadOnly(true);
    infoEdit->setMinimumHeight(180);
    infoEdit->setPlaceholderText("Local addresses and reachability guidance will appear here...");

    refreshBtn = new QPushButton("Refresh Network Info");
    refreshBtn->setMinimumHeight(BTN_HEIGHT);
    connect(refreshBtn, &QPushButton::clicked, this, &NetworkInfoPanel::refreshNetworkInfo);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(refreshBtn);
    btnLayout->addStretch();

    layout->addWidget(infoEdit);
    layout->addLayout(btnLayout);
    mainLayout->addWidget(group);
}

void NetworkInfoPanel::applyStyles()
{
    refreshBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #0d6efd; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #0b5ed7; }"
    );
}

void NetworkInfoPanel::setupAccessibility()
{
    infoEdit->setAccessibleName("Network Information");
    infoEdit->setAccessibleDescription("Displays local network addresses and guidance for LAN and internet access");

    refreshBtn->setAccessibleName("Refresh Network Information");
    refreshBtn->setAccessibleDescription("Refreshes the list of local interfaces and reachability guidance");
}

void NetworkInfoPanel::updateServerContext(const QString &protocol, const QString &bindAddress, int port)
{
    currentProtocol = protocol;
    currentBindAddress = bindAddress;
    currentPort = port;
    refreshNetworkInfo();
}

void NetworkInfoPanel::refreshNetworkInfo()
{
    infoEdit->setPlainText(buildNetworkSummary());
}

QString NetworkInfoPanel::buildNetworkSummary() const
{
    QString summary;
    summary += "Server Context\n";
    summary += "-------------\n";
    summary += QString("Protocol: %1\n").arg(formatProtocolName(currentProtocol));
    summary += QString("Bind Address: %1\n").arg(currentBindAddress);
    summary += QString("Port: %1\n\n").arg(currentPort);

    summary += buildAddressSummary();
    summary += "\n";
    summary += buildReachabilitySummary();
    return summary;
}

QString NetworkInfoPanel::buildAddressSummary() const
{
    QString summary;
    summary += "Local Interfaces\n";
    summary += "---------------\n";
    summary += QString("Hostname: %1\n").arg(QHostInfo::localHostName());

    QStringList addresses;
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
            if (ipString.isEmpty() || ipString == "0.0.0.0") {
                continue;
            }

            addresses.append(QString("%1: %2").arg(iface.humanReadableName(), ipString));
        }
    }

    addresses.removeDuplicates();

    if (addresses.isEmpty()) {
        summary += "No active IPv4 interfaces detected.\n";
    } else {
        for (const QString &address : addresses) {
            summary += QString("- %1\n").arg(address);
        }
    }

    return summary;
}

QString NetworkInfoPanel::buildReachabilitySummary() const
{
    QString summary;
    summary += "Reachability Guidance\n";
    summary += "---------------------\n";

    const QString scheme = protocolSchemeHint();
    const bool loopbackOnly = (currentBindAddress == "127.0.0.1");
    const bool allInterfaces = (currentBindAddress == "0.0.0.0");

    if (loopbackOnly) {
        summary += "This server is bound to loopback only.\n";
        summary += "Only clients on the same machine can connect.\n";
        summary += QString("Example: %1://127.0.0.1:%2\n\n").arg(scheme).arg(currentPort);
        summary += "To allow LAN or internet clients, choose All Interfaces or a specific local IP.\n";
        return summary;
    }

    if (allInterfaces) {
        summary += "This server will listen on all available IPv4 interfaces.\n";
    } else {
        summary += QString("This server will listen only on %1.\n").arg(currentBindAddress);
    }

    summary += "\nLAN access:\n";
    if (allInterfaces) {
        summary += QString("- Use one of the local interface IPs above with %1://<local-ip>:%2\n")
                       .arg(scheme)
                       .arg(currentPort);
    } else {
        summary += QString("- Use %1://%2:%3 from another device on the same network\n")
                       .arg(scheme, currentBindAddress)
                       .arg(currentPort);
    }

    summary += "\nInternet access:\n";
    summary += "- Open the same port in the OS firewall.\n";
    summary += "- Forward the router port to this machine if you are behind NAT.\n";
    summary += "- Use your public IP or domain name from remote clients.\n";
    summary += "- If your ISP uses CGNAT, normal port forwarding may not work.\n";
    summary += "- For public internet use, add TLS and authentication before relying on this outside trusted networks.\n";

    return summary;
}

QString NetworkInfoPanel::protocolSchemeHint() const
{
    if (currentProtocol == "WebSocket Server") {
        return "ws";
    }
    if (currentProtocol == "HTTP Server") {
        return "http";
    }
    if (currentProtocol == "UDP Server") {
        return "udp";
    }
    return "tcp";
}
