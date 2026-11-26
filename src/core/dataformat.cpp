#include "commlink/core/dataformat.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTextStream>
#include <QDebug>
#include <QMetaType>
#include <QRegularExpression>

DataMessage::DataMessage(DataFormatType t, const QVariant& d) : type(t), data(d) {}

QByteArray DataMessage::serialize() const {
    if (hasRawBytes)
        return rawBytes;
    switch (type) {
    case DataFormatType::JSON: {
        if (data.canConvert<QJsonDocument>()) {
            QJsonDocument doc = data.value<QJsonDocument>();
            return doc.toJson(QJsonDocument::Compact);
        }
        return QByteArray();
    }
    case DataFormatType::XML: {
        // Simple XML serialization
        QString xml = data.toString();
        return xml.toUtf8();
    }
    case DataFormatType::CSV: {
        // Simple CSV
        return data.toString().toUtf8();
    }
    case DataFormatType::TEXT: {
        return data.toString().toUtf8();
    }
    case DataFormatType::BINARY: {
        if (data.type() == QVariant::ByteArray) {
            return data.toByteArray();
        }
        return QByteArray();
    }
    case DataFormatType::HEX: {
        if (data.type() == QVariant::ByteArray) {
            return data.toByteArray();
        }
        return QByteArray();
    }
    default:
        return QByteArray();
    }
}

DataMessage DataMessage::deserialize(const QByteArray& bytes, DataFormatType type) {
    DataMessage msg(type);
    msg.rawBytes = bytes;
    msg.wireBytes = bytes;
    msg.hasRawBytes = true;
    switch (type) {
    case DataFormatType::JSON: {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);
        if (error.error == QJsonParseError::NoError) {
            msg.data = doc;
        } else {
            msg.decodeError = QString("JSON at byte %1: %2").arg(error.offset).arg(error.errorString());
            // If JSON parsing fails, store as text
            msg.data = QString::fromUtf8(bytes);
        }
        break;
    }
    case DataFormatType::XML: {
        QString xmlString = QString::fromUtf8(bytes);
        msg.data = xmlString;
        break;
    }
    case DataFormatType::CSV: {
        QString csvString = QString::fromUtf8(bytes);
        msg.data = csvString;
        break;
    }
    case DataFormatType::TEXT: {
        QString textString = QString::fromUtf8(bytes);
        msg.data = textString;
        break;
    }
    case DataFormatType::BINARY: {
        msg.data = bytes;
        break;
    }
    case DataFormatType::HEX: {
        msg.data = bytes;
        break;
    }
    }
    return msg;
}

QString DataMessage::toDisplayString() const {
    switch (type) {
    case DataFormatType::JSON: {
        if (data.canConvert<QJsonDocument>()) {
            QJsonDocument doc = data.value<QJsonDocument>();
            return doc.toJson(QJsonDocument::Indented);
        }
        QString str = data.toString();
        return str.isEmpty() ? "[Empty JSON]" : str;
    }
    case DataFormatType::XML: {
        if (data.type() == QVariant::String) {
            QString str = data.toString();
            return str.isEmpty() ? "[Empty XML]" : str;
        }
        return data.toString();
    }
    case DataFormatType::CSV: {
        if (data.type() == QVariant::String) {
            QString str = data.toString();
            return str.isEmpty() ? "[Empty CSV]" : str;
        }
        return data.toString();
    }
    case DataFormatType::TEXT: {
        if (data.type() == QVariant::String) {
            QString str = data.toString();
            return str.isEmpty() ? "[Empty Text]" : str;
        }
        return data.toString();
    }
    case DataFormatType::BINARY: {
        if (data.type() == QVariant::ByteArray) {
            QByteArray bytes = data.toByteArray();
            return QString("Binary data (%1 bytes): %2").arg(bytes.size()).arg(QString::fromLatin1(bytes.toHex()));
        }
        return "Binary data";
    }
    case DataFormatType::HEX: {
        if (data.type() == QVariant::ByteArray) {
            return QString::fromLatin1(data.toByteArray().toHex());
        }
        return "Hex data";
    }
    default:
        return data.toString();
    }
}

bool DataMessage::validateInput(const QString& input, DataFormatType type) {
    switch (type) {
    case DataFormatType::JSON: {
        QJsonParseError error;
        QJsonDocument::fromJson(input.toUtf8(), &error);
        return error.error == QJsonParseError::NoError;
    }
    case DataFormatType::XML: {
        QXmlStreamReader reader(input);
        while (!reader.atEnd())
            reader.readNext();
        return !reader.hasError() && !input.trimmed().isEmpty();
    }
    case DataFormatType::CSV:
        return !input.isEmpty();
    case DataFormatType::TEXT:
        return true;
    case DataFormatType::BINARY:
    case DataFormatType::HEX: {
        QRegularExpression hexPattern("^[0-9A-Fa-f\\s]*$");
        QString compact = input;
        compact.remove(QRegularExpression("\\s"));
        return !compact.isEmpty() && compact.size() % 2 == 0 && hexPattern.match(input).hasMatch();
    }
    default:
        return false;
    }
}

QVariant DataMessage::parseInput(const QString& input, DataFormatType type) {
    switch (type) {
    case DataFormatType::JSON: {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            return doc;
        }
        return QVariant();
    }
    case DataFormatType::XML:
    case DataFormatType::CSV:
    case DataFormatType::TEXT:
        return input;
    case DataFormatType::BINARY:
        return QByteArray::fromHex(input.toUtf8());
    case DataFormatType::HEX:
        return QByteArray::fromHex(input.toUtf8());
    default:
        return QVariant();
    }
}

// Register DataMessage with Qt's meta-object system
static int dataMessageTypeId = qRegisterMetaType<DataMessage>("DataMessage");
