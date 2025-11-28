#include "commlink/core/dataformat.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTextStream>
#include <QDebug>
#include <QMetaType>

DataMessage::DataMessage(DataFormatType t, const QVariant& d) : type(t), data(d) {}

QByteArray DataMessage::serialize() const {
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
        QString xml = "<message>" + data.toString() + "</message>";
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
            return data.toByteArray().toHex();
        }
        return QByteArray();
    }
    default:
        return QByteArray();
    }
}

DataMessage DataMessage::deserialize(const QByteArray& bytes, DataFormatType type) {
    DataMessage msg(type);
    switch (type) {
    case DataFormatType::JSON: {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);
        if (error.error == QJsonParseError::NoError) {
            msg.data = doc;
        }
        break;
    }
    case DataFormatType::XML: {
        msg.data = QString::fromUtf8(bytes);
        break;
    }
    case DataFormatType::CSV: {
        msg.data = QString::fromUtf8(bytes);
        break;
    }
    case DataFormatType::TEXT: {
        msg.data = QString::fromUtf8(bytes);
        break;
    }
    case DataFormatType::BINARY: {
        msg.data = bytes;
        break;
    }
    case DataFormatType::HEX: {
        msg.data = QByteArray::fromHex(bytes);
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
        return data.toString();
    }
    case DataFormatType::XML:
    case DataFormatType::CSV:
    case DataFormatType::TEXT:
        return data.toString();
    case DataFormatType::BINARY: {
        if (data.type() == QVariant::ByteArray) {
            return QString("Binary data (%1 bytes)").arg(data.toByteArray().size());
        }
        return "Binary data";
    }
    case DataFormatType::HEX: {
        if (data.type() == QVariant::ByteArray) {
            return data.toByteArray().toHex();
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
    case DataFormatType::XML:
        return !input.isEmpty(); // Basic check
    case DataFormatType::CSV:
        return !input.isEmpty();
    case DataFormatType::TEXT:
        return true;
    case DataFormatType::BINARY:
        return !input.isEmpty(); // Assume hex input
    case DataFormatType::HEX: {
        return !input.isEmpty() && input.contains(QRegExp("^[0-9A-Fa-f\\s]*$"));
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
