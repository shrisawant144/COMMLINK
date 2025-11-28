#ifndef DATAFORMAT_H
#define DATAFORMAT_H

#include <QByteArray>
#include <QVariant>
#include <QString>

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

    DataMessage(DataFormatType t = DataFormatType::TEXT, const QVariant& d = QVariant());
    
    QByteArray serialize() const;
    static DataMessage deserialize(const QByteArray& bytes, DataFormatType type);
    QString toDisplayString() const;
    
    static bool validateInput(const QString& input, DataFormatType type);
    static QVariant parseInput(const QString& input, DataFormatType type);
};

Q_DECLARE_METATYPE(DataMessage)

#endif // DATAFORMAT_H
