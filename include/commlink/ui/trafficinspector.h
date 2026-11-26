#pragma once
#include "../core/dataformat.h"

#include <QJsonArray>
#include <QLineEdit>
#include <QTableWidget>
#include <QTextEdit>
#include <QWidget>

class TrafficInspector : public QWidget {
    Q_OBJECT
public:
    explicit TrafficInspector(QWidget* parent = nullptr);
    void append(const DataMessage& message, const QString& direction, const QString& protocol,
                const QString& peer, const QString& detail = QString(), qint64 elapsed = -1);
    void clear();
    QJsonArray records() const;
signals:
    void replayRequested(const QByteArray& payload, int format);

private:
    struct Record {
        DataMessage message;
        QString direction, protocol, peer, detail, timestamp;
        qint64 elapsed;
    };
    QList<Record> entries;
    qint64 retainedBytes = 0;
    QTableWidget* table;
    QLineEdit* filter;
    QTextEdit *text, *hex, *formatted, *wire;
    void selectRecord();
    void applyFilter();
};
