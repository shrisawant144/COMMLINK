#include "commlink/ui/trafficinspector.h"

#include <QDateTime>
#include <QFileDialog>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QTabWidget>
#include <QVBoxLayout>

TrafficInspector::TrafficInspector(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    filter = new QLineEdit;
    filter->setPlaceholderText("Filter direction, protocol, endpoint or content...");
    layout->addWidget(filter);
    table = new QTableWidget(0, 6);
    table->setHorizontalHeaderLabels(
        {"Time", "Direction", "Protocol", "Endpoint", "Bytes", "Elapsed ms"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setDefaultSectionSize(120);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table, 2);
    auto* details = new QTabWidget;
    text = new QTextEdit;
    hex = new QTextEdit;
    formatted = new QTextEdit;
    wire = new QTextEdit;
    for (auto* edit : {text, hex, formatted, wire}) {
        edit->setReadOnly(true);
        edit->setMinimumHeight(80);
    }
    details->addTab(text, "Text");
    details->addTab(hex, "Hex");
    details->addTab(formatted, "Decoded");
    details->addTab(wire, "Wire / Details");
    details->setUsesScrollButtons(true);
    layout->addWidget(details, 1);
    auto* buttons = new QHBoxLayout;
    auto* replay = new QPushButton("Use Payload");
    auto* exportButton = new QPushButton("Export Conversation");
    buttons->addWidget(replay);
    buttons->addWidget(exportButton);
    buttons->addStretch();
    layout->addLayout(buttons);
    connect(table, &QTableWidget::itemSelectionChanged, this, &TrafficInspector::selectRecord);
    connect(filter, &QLineEdit::textChanged, this, &TrafficInspector::applyFilter);
    connect(replay, &QPushButton::clicked, this, [this] {
        const int row = table->currentRow();
        if (row >= 0 && row < entries.size())
            emit replayRequested(entries[row].message.serialize(),
                                 static_cast<int>(entries[row].message.type));
    });
    connect(exportButton, &QPushButton::clicked, this, [this] {
        const auto filename = QFileDialog::getSaveFileName(this, "Export Conversation",
                                                           "conversation.json", "JSON (*.json)");
        if (filename.isEmpty())
            return;
        QSaveFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, "Export", file.errorString());
            return;
        }
        const auto bytes =
            QJsonDocument(QJsonObject{{"version", 1}, {"records", records()}}).toJson();
        if (file.write(bytes) != bytes.size() || !file.commit())
            QMessageBox::warning(this, "Export", file.errorString());
    });
}
void TrafficInspector::append(const DataMessage& message, const QString& direction,
                              const QString& protocol, const QString& peer, const QString& detail,
                              qint64 elapsed, const QString& timestamp) {
    const auto payload = message.serialize();
    const auto originalWire = message.wireBytes.isNull() ? payload : message.wireBytes;
    if (payload.size() + originalWire.size() > 32 * 1024 * 1024)
        return;
    retainedBytes += payload.size() + originalWire.size();
    while (!entries.isEmpty() && (entries.size() >= 2000 || retainedBytes > 64 * 1024 * 1024)) {
        retainedBytes -=
            entries.first().message.serialize().size() + entries.first().message.wireBytes.size();
        entries.removeFirst();
        table->removeRow(0);
    }
    DataMessage retained = message;
    retained.wireBytes = originalWire;
    entries.append({retained, direction, protocol, peer, detail,
                    timestamp.isEmpty()
                        ? QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)
                        : timestamp,
                    elapsed});
    const auto& entry = entries.last();
    const int row = table->rowCount();
    table->insertRow(row);
    const QStringList fields{entry.timestamp,
                             direction,
                             protocol,
                             peer,
                             QString::number(payload.size()),
                             elapsed < 0 ? QString() : QString::number(elapsed)};
    for (int col = 0; col < fields.size(); ++col)
        table->setItem(row, col, new QTableWidgetItem(fields[col]));
    const auto query = filter->text();
    table->setRowHidden(row, !query.isEmpty() &&
                                 !(direction + ' ' + protocol + ' ' + peer + ' ' + detail)
                                      .contains(query, Qt::CaseInsensitive) &&
                                 !QString::fromUtf8(payload).contains(query, Qt::CaseInsensitive));
}
void TrafficInspector::selectRecord() {
    const int row = table->currentRow();
    if (row < 0 || row >= entries.size()) {
        for (auto* edit : {text, hex, formatted, wire})
            edit->clear();
        return;
    }
    const auto& entry = entries[row];
    const auto payload = entry.message.serialize();
    text->setPlainText(QString::fromUtf8(payload));
    hex->setPlainText(QString::fromLatin1(payload.toHex(' ')));
    formatted->setPlainText((entry.message.decodeError.isEmpty() ? QString() : "Decode error: " + entry.message.decodeError + "\n\n") + entry.message.toDisplayString());
    const QString scope =
        entry.protocol == "TCP" ? "TCP stream chunk / framed message"
        : entry.protocol == "UDP"
            ? "UDP datagram payload"
            : "Application payload; HTTP/WebSocket framing and TLS are handled by Qt";
    wire->setPlainText(scope + "\n" + entry.detail + "\n" + entry.timestamp + "\n" +
                       QString::fromLatin1(entry.message.wireBytes.toHex(' ')));
}
void TrafficInspector::applyFilter() {
    const auto query = filter->text();
    for (int row = 0; row < entries.size(); ++row) {
        const auto& e = entries[row];
        table->setRowHidden(
            row,
            !query.isEmpty() &&
                !(e.direction + ' ' + e.protocol + ' ' + e.peer + ' ' + e.detail)
                     .contains(query, Qt::CaseInsensitive) &&
                !QString::fromUtf8(e.message.serialize()).contains(query, Qt::CaseInsensitive));
    }
}
void TrafficInspector::clear() {
    table->setRowCount(0);
    entries.clear();
    retainedBytes = 0;
    for (auto* edit : {text, hex, formatted, wire})
        edit->clear();
}
QJsonArray TrafficInspector::records() const {
    QJsonArray result;
    for (const auto& e : entries)
        result.append(
            QJsonObject{{"timestamp", e.timestamp},
                        {"direction", e.direction},
                        {"protocol", e.protocol},
                        {"endpoint", e.peer},
                        {"format", static_cast<int>(e.message.type)},
                        {"payloadBase64", QString::fromLatin1(e.message.serialize().toBase64())},
                        {"wireBase64", QString::fromLatin1(e.message.wireBytes.toBase64())},
                        {"elapsedMs", double(e.elapsed)},
                        {"details", e.detail}});
    return result;
}
