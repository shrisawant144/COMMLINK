#include "exportmanager.h"
#include <QFile>
#include <QTextStream>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QStandardPaths>
#include <QIODevice>
#include <QDateTime>
#include <QMessageBox>

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

bool ExportManager::exportMessages(const QList<QJsonDocument>& messages, const QString& format, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    if (format == "json") {
        QJsonArray array;
        for (const QJsonDocument& doc : messages) {
            array.append(doc.object());
        }
        QJsonDocument exportDoc(array);
        out << exportDoc.toJson();
    } else if (format == "txt") {
        for (const QJsonDocument& doc : messages) {
            out << doc.toJson(QJsonDocument::Indented) << "\n";
        }
    } else if (format == "csv") {
        out << "Message\n";
        for (const QJsonDocument& doc : messages) {
            out << "\"" << doc.toJson(QJsonDocument::Compact) << "\"\n";
        }
    }
    return true;
}

QStringList ExportManager::getSupportedFormats() {
    return {"txt", "csv", "json"};
}
