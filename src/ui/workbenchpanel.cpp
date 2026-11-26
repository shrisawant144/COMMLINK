#include "commlink/ui/workbenchpanel.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

WorkbenchPanel::WorkbenchPanel(QWidget* parent) : QWidget(parent) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* group = new QGroupBox("Workspace");
    auto* layout = new QVBoxLayout(group);
    profiles = new QComboBox;
    profiles->setEditable(true);
    profiles->setAccessibleName("Workspace Name");
    profiles->setPlaceholderText("Workspace name");
    profiles->lineEdit()->setPlaceholderText("Workspace name");
    layout->addWidget(profiles);
    auto* buttons = new QHBoxLayout;
    auto* save = new QPushButton("Save");
    auto* open = new QPushButton("Open");
    optionsButton = new QPushButton("Options");
    replayButton = new QPushButton("Replay");
    stopButton = new QPushButton("Stop");
    stopButton->setEnabled(false);
    replayStatus = new QLabel("Replay ready");
    replayStatus->setWordWrap(true);
    buttons->addWidget(save);
    buttons->addWidget(open);
    buttons->addWidget(optionsButton);
    buttons->addWidget(replayButton);
    layout->addLayout(buttons);
    outer->addWidget(group);
    auto* status = new QHBoxLayout;
    status->addWidget(replayStatus, 1);
    status->addWidget(stopButton);
    layout->addLayout(status);
    connect(replayButton, &QPushButton::clicked, this, &WorkbenchPanel::editReplay);
    connect(stopButton, &QPushButton::clicked, this, &WorkbenchPanel::stopReplayRequested);
    connect(save, &QPushButton::clicked, this, [this] { emit saveRequested(profileName()); });
    connect(open, &QPushButton::clicked, this, [this] { emit openRequested(profileName()); });
    connect(optionsButton, &QPushButton::clicked, this, &WorkbenchPanel::editOptions);
}
QString WorkbenchPanel::profileName() const {
    return profiles->currentText().trimmed();
}
void WorkbenchPanel::setProfiles(const QStringList& names) {
    const auto name = profiles->currentText();
    profiles->clear();
    profiles->addItems(names);
    profiles->setCurrentText(name);
}

void WorkbenchPanel::setConnectionActive(bool active) {
    optionsButton->setEnabled(!active);
}
void WorkbenchPanel::setReplayState(bool active, const QString& message) {
    replayButton->setEnabled(!active);
    stopButton->setEnabled(active);
    replayStatus->setText(message);
}
void WorkbenchPanel::editOptions() {
    QDialog dialog(this);
    dialog.setWindowTitle("Workspace Options");
    dialog.resize(550, 540);
    auto* layout = new QVBoxLayout(&dialog);
    auto* tabs = new QTabWidget;
    layout->addWidget(tabs);
    auto* tcpPage = new QWidget;
    auto* tcp = new QFormLayout(tcpPage);
    auto* framing = new QComboBox;
    framing->addItems(
        {"Raw stream chunks", "Newline", "Custom delimiter", "32-bit length (network order)"});
    framing->setCurrentIndex(qBound(0, settings["framing"].toInt(), 3));
    auto* delimiter = new QLineEdit(settings["delimiter"].toString("0a"));
    auto* keepAlive = new QCheckBox("Enable TCP keep-alive");
    keepAlive->setChecked(settings["keepAlive"].toBool());
    tcp->addRow("TCP framing:", framing);
    tcp->addRow("Delimiter (hex):", delimiter);
    tcp->addRow(keepAlive);
    auto* note = new QLabel("Raw mode displays stream chunks, which do not identify complete "
                            "application messages. Framing applies to both sending and receiving.");
    note->setWordWrap(true);
    tcp->addRow(note);
    tabs->addTab(tcpPage, "TCP");
    auto* httpPage = new QWidget;
    auto* http = new QFormLayout(httpPage);
    auto* timeout = new QSpinBox;
    timeout->setRange(100, 300000);
    timeout->setValue(settings["timeoutMs"].toInt(30000));
    auto* headers = new QTextEdit;
    headers->setAcceptRichText(false);
    headers->setPlaceholderText("X-Request-ID: demo\nAccept: application/json");
    headers->setPlainText(settings["headers"].toString());
    auto* auth = new QComboBox;
    auth->addItems({"None", "Bearer token", "Basic"});
    auto* user = new QLineEdit;
    auto* secret = new QLineEdit;
    secret->setEchoMode(QLineEdit::Password);
    if (authorization.startsWith("Bearer ")) {
        auth->setCurrentIndex(1);
        secret->setText(authorization.mid(7));
    }
    if (authorization.startsWith("Basic ")) {
        auth->setCurrentIndex(2);
        const auto value =
            QString::fromUtf8(QByteArray::fromBase64(authorization.mid(6).toLatin1()));
        const int colon = value.indexOf(':');
        user->setText(value.left(colon));
        secret->setText(value.mid(colon + 1));
    }
    http->addRow("Timeout (ms):", timeout);
    http->addRow("Headers:", headers);
    http->addRow("Authentication:", auth);
    http->addRow("Username (Basic):", user);
    http->addRow("Token / Password:", secret);
    auto* authNote = new QLabel("Authentication fields are kept only for this application session. "
                                "TLS certificates are verified using the system trust store.");
    authNote->setWordWrap(true);
    http->addRow(authNote);
    tabs->addTab(httpPage, "HTTP");
    auto* wsPage = new QWidget;
    auto* ws = new QFormLayout(wsPage);
    auto* ping = new QSpinBox;
    ping->setRange(0, 300000);
    ping->setSingleStep(1000);
    ping->setValue(settings["pingMs"].toInt());
    ws->addRow("Ping interval (ms; 0 off):", ping);
    tabs->addTab(wsPage, "WebSocket");
    auto* varsPage = new QWidget;
    auto* varsLayout = new QVBoxLayout(varsPage);
    auto* variables = new QTextEdit;
    variables->setAcceptRichText(false);
    QStringList entries;
    const auto values = settings["variables"].toObject();
    for (auto it = values.begin(); it != values.end(); ++it)
        entries << it.key() + "=" + it.value().toString();
    variables->setPlainText(entries.join('\n'));
    variables->setPlaceholderText("HOST=127.0.0.1\nUse ${HOST} in addresses, headers and payloads");
    varsLayout->addWidget(new QLabel("Variables (NAME=value):"));
    varsLayout->addWidget(variables);
    auto* retention = new QSpinBox;
    retention->setRange(0, 3650);
    retention->setValue(settings["retentionDays"].toInt());
    retention->setSuffix(" days (0 keeps all)");
    varsLayout->addWidget(new QLabel("Delete history older than (applied on startup):"));
    varsLayout->addWidget(retention);
    tabs->addTab(varsPage, "Variables / History");
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted)
        return;
    const auto delimiterText = delimiter->text().remove(QRegularExpression("\\s"));
    if (framing->currentIndex() == 2 &&
        (delimiterText.isEmpty() || delimiterText.size() % 2 || delimiterText.size() > 64 ||
         !QRegularExpression("^[0-9a-fA-F]+$").match(delimiterText).hasMatch())) {
        QMessageBox::warning(this, "Delimiter",
                             "Enter a non-empty delimiter as complete hexadecimal bytes.");
        return;
    }
    for (const auto& line : headers->toPlainText().split('\n')) {
        if (line.trimmed().isEmpty())
            continue;
        const int colon = line.indexOf(':');
        if (colon <= 0 || line.contains('\r') ||
            !QRegularExpression("^[!#$%&'*+.^_`|~0-9A-Za-z-]+$")
                 .match(line.left(colon))
                 .hasMatch()) {
            QMessageBox::warning(this, "Headers", "Use one valid Header-Name: value per line.");
            return;
        }
    }
    settings["framing"] = framing->currentIndex();
    settings["delimiter"] = delimiterText;
    settings["keepAlive"] = keepAlive->isChecked();
    settings["headers"] = headers->toPlainText();
    settings["timeoutMs"] = timeout->value();
    settings["pingMs"] = ping->value();
    settings["retentionDays"] = retention->value();
    if (auth->currentIndex() == 0)
        authorization.clear();
    if (auth->currentIndex() == 1)
        authorization = "Bearer " + secret->text();
    if (auth->currentIndex() == 2)
        authorization = "Basic " + QString::fromLatin1(
                                       (user->text() + ':' + secret->text()).toUtf8().toBase64());
    QJsonObject vars;
    for (const auto& line : variables->toPlainText().split('\n')) {
        const int eq = line.indexOf('=');
        if (eq > 0)
            vars[line.left(eq).trimmed()] = line.mid(eq + 1);
    }
    settings["variables"] = vars;
    emit optionsChanged();
}
void WorkbenchPanel::editReplay() {
    QDialog dialog(this);
    dialog.setWindowTitle("Replay Current Payload");
    auto* layout = new QFormLayout(&dialog);
    const auto previous = settings["replay"].toObject();
    auto* count = new QSpinBox;
    count->setRange(1, 10000);
    count->setValue(previous["count"].toInt(1));
    auto* interval = new QSpinBox;
    interval->setRange(10, 60000);
    interval->setValue(previous["intervalMs"].toInt(1000));
    auto* timeout = new QSpinBox;
    timeout->setRange(100, 300000);
    timeout->setValue(previous["timeoutMs"].toInt(5000));
    auto* assertion = new QComboBox;
    assertion->addItems(
        {"None", "Response contains text", "Exact response bytes (hex)", "JSON field equals"});
    assertion->setCurrentIndex(previous["assertion"].toInt());
    auto* path = new QLineEdit(previous["path"].toString());
    path->setPlaceholderText("result.status (arrays use numeric segments)");
    auto* expected = new QLineEdit(previous["expected"].toString());
    layout->addRow("Send count:", count);
    layout->addRow("Interval (ms):", interval);
    layout->addRow("Response timeout (ms):", timeout);
    layout->addRow("Assertion:", assertion);
    layout->addRow("JSON field path:", path);
    layout->addRow("Expected value:", expected);
    auto* note = new QLabel("Replays the current payload to the current destination. Assertions "
                            "inspect the first response from that transport after each send.");
    note->setWordWrap(true);
    layout->addRow(note);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted)
        return;
    QJsonObject config{{"count", count->value()},       {"intervalMs", interval->value()},
                       {"timeoutMs", timeout->value()}, {"assertion", assertion->currentIndex()},
                       {"path", path->text()},          {"expected", expected->text()}};
    settings["replay"] = config;
    emit replayRequested(config);
}
