#include "commlink/core/exportmanager.h"
#include <QFile>
#include <QTextStream>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QStandardPaths>
#include <QIODevice>
#include <QDateTime>

bool ExportManager::exportLogs(const QStringList& logs, const QString& format, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    if (format == "txt") {
        for (const QString& log : logs) {
            out << log << "\n";
        }
    } else if (format == "csv") {
        out << "Message\n";
        for (const QString& log : logs) {
            out << "\"" << log << "\"\n";
        }
    }
    return true;
}

bool ExportManager::exportMessages(const QList<DataMessage>& messages, const QString& format, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    if (format == "json") {
        QJsonArray array;
        for (const DataMessage& msg : messages) {
            if (msg.type == DataFormatType::JSON && msg.data.canConvert<QJsonDocument>()) {
                array.append(msg.data.value<QJsonDocument>().object());
            } else {
                // Convert other formats to JSON representation
                QJsonObject obj;
                // Convert enum type to string (formatTypeToString was not available)
                QString typeStr;
                switch (msg.type) {
                case DataFormatType::JSON: typeStr = "JSON"; break;
                case DataFormatType::XML: typeStr = "XML"; break;
                case DataFormatType::CSV: typeStr = "CSV"; break;
                case DataFormatType::TEXT: typeStr = "TEXT"; break;
                case DataFormatType::BINARY: typeStr = "BINARY"; break;
                case DataFormatType::HEX: typeStr = "HEX"; break;
                default: typeStr = "UNKNOWN"; break;
                }
                obj["type"] = typeStr;
                obj["data"] = msg.toDisplayString();
                array.append(obj);
            }
        }
        QJsonDocument exportDoc(array);
        out << exportDoc.toJson();
    } else if (format == "txt") {
        for (const DataMessage& msg : messages) {
            out << msg.toDisplayString() << "\n";
        }
    } else if (format == "csv") {
        out << "Type,Data\n";
        for (const DataMessage& msg : messages) {
            QString typeStr;
            switch (msg.type) {
            case DataFormatType::JSON: typeStr = "JSON"; break;
            case DataFormatType::XML: typeStr = "XML"; break;
            case DataFormatType::CSV: typeStr = "CSV"; break;
            case DataFormatType::TEXT: typeStr = "TEXT"; break;
            case DataFormatType::BINARY: typeStr = "BINARY"; break;
            case DataFormatType::HEX: typeStr = "HEX"; break;
            }
            QString dataStr = msg.toDisplayString().replace("\"", "\"\"");
            out << "\"" << typeStr << "\",\"" << dataStr << "\"\n";
        }
    }
    return true;
}

QStringList ExportManager::getSupportedFormats() {
    return {"txt", "csv", "json"};
}
