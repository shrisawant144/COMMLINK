#include "commlink/ui/workbenchpanel.h"
#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QVBoxLayout>

WorkbenchPanel::WorkbenchPanel(QWidget *parent) : QWidget(parent) {
    auto *outer = new QVBoxLayout(this); outer->setContentsMargins(0,0,0,0);
    auto *group = new QGroupBox("Workspace"); auto *layout = new QVBoxLayout(group);
    profiles = new QComboBox; profiles->setEditable(true); profiles->setAccessibleName("Workspace Name");
    profiles->setPlaceholderText("Workspace name");
    layout->addWidget(profiles);
    auto *buttons = new QHBoxLayout;
    auto *save = new QPushButton("Save"); auto *open = new QPushButton("Open"); auto *options = new QPushButton("Options");
    buttons->addWidget(save); buttons->addWidget(open); buttons->addWidget(options); layout->addLayout(buttons); outer->addWidget(group);
    connect(save, &QPushButton::clicked, this, [this]{ emit saveRequested(profileName()); });
    connect(open, &QPushButton::clicked, this, [this]{ emit openRequested(profileName()); });
    connect(options, &QPushButton::clicked, this, &WorkbenchPanel::editOptions);
}
QString WorkbenchPanel::profileName() const { return profiles->currentText().trimmed(); }
void WorkbenchPanel::setProfiles(const QStringList &names) {
    const auto name = profiles->currentText(); profiles->clear(); profiles->addItems(names); profiles->setCurrentText(name);
}
void WorkbenchPanel::editOptions() {
    QDialog dialog(this); dialog.setWindowTitle("Workspace Options"); dialog.resize(540, 520);
    auto *layout = new QVBoxLayout(&dialog); auto *form = new QFormLayout;
    auto *framing = new QComboBox; framing->addItems({"Raw stream chunks", "Newline", "Custom delimiter", "32-bit length (network order)"});
    framing->setCurrentIndex(settings["framing"].toInt());
    auto *delimiter = new QLineEdit(settings["delimiter"].toString("0a"));
    auto *keepAlive = new QCheckBox("Enable TCP keep-alive"); keepAlive->setChecked(settings["keepAlive"].toBool());
    auto *variables = new QTextEdit; variables->setAcceptRichText(false);
    QStringList entries; const auto values = settings["variables"].toObject();
    for (auto it = values.begin(); it != values.end(); ++it) entries << it.key() + "=" + it.value().toString();
    variables->setPlainText(entries.join('\n')); variables->setPlaceholderText("HOST=127.0.0.1\nUse ${HOST} in addresses and payloads");
    form->addRow("TCP framing:", framing); form->addRow("Delimiter (hex):", delimiter); form->addRow(keepAlive);
    layout->addLayout(form); layout->addWidget(new QLabel("Workspace variables (NAME=value):")); layout->addWidget(variables);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons); connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;
    settings["framing"] = framing->currentIndex(); settings["delimiter"] = delimiter->text(); settings["keepAlive"] = keepAlive->isChecked();
    QJsonObject vars;
    for (const auto &line : variables->toPlainText().split('\n')) { const int eq = line.indexOf('='); if (eq > 0) vars[line.left(eq).trimmed()] = line.mid(eq + 1); }
    settings["variables"] = vars; emit optionsChanged();
}
