#ifndef DATAFORMAT_H
#define DATAFORMAT_H

#include <QtCore/QVariant>
#include <QtCore/QByteArray>
#include <QtCore/QString>

enum class DataFormatType {
    JSON,
    XML,
    CSV,
    TEXT,
    BINARY,
    HEX
};

class DataMessage {
public:
    DataFormatType type;
    QVariant data;

    DataMessage(DataFormatType t = DataFormatType::JSON, const QVariant& d = QVariant());

    // Serialize to bytes for sending
    QByteArray serialize() const;

    // Deserialize from bytes for receiving
    static DataMessage deserialize(const QByteArray& bytes, DataFormatType type);

    // Convert to display string
    QString toDisplayString() const;

    // Validate input string for the format
    static bool validateInput(const QString& input, DataFormatType type);

    // Parse input string to data
    static QVariant parseInput(const QString& input, DataFormatType type);
};

#endif // DATAFORMAT_H
