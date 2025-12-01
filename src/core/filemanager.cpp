#include "commlink/core/filemanager.h"
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QSettings>
#include <QtCore/QJsonDocument>
#include <QtCore/QIODevice>
#include <QtWidgets/QMessageBox>

QString FileManager::loadMessageFromFile(const QString& filePath, DataFormatType format) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Validate based on format
    if (!DataMessage::validateInput(content, format)) {
        return QString();
    }

    return content;
}

bool FileManager::saveMessageToFile(const QString& content, const QString& filePath, DataFormatType format) {
    Q_UNUSED(format);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QByteArray data = content.toUtf8();
    qint64 bytesWritten = file.write(data);
    if (bytesWritten == -1 || bytesWritten != data.size()) {
        file.close();
        return false;
    }
    if (!file.flush()) {
        file.close();
        return false;
    }
    file.close();
    addToRecentFiles(filePath);
    return true;
}

QString FileManager::getFileExtension(DataFormatType format) {
    switch (format) {
    case DataFormatType::JSON:
        return "json";
    case DataFormatType::XML:
        return "xml";
    case DataFormatType::CSV:
        return "csv";
    case DataFormatType::TEXT:
        return "txt";
    case DataFormatType::BINARY:
        return "bin";
    case DataFormatType::HEX:
        return "hex";
    default:
        return "txt";
    }
}

QStringList FileManager::getRecentFiles() {
    QSettings settings;
    return settings.value("recentFiles").toStringList();
}

void FileManager::addToRecentFiles(const QString& filePath) {
    QSettings settings;
    QStringList recent = getRecentFiles();
    recent.removeAll(filePath);
    recent.prepend(filePath);
    if (recent.size() > 10) {
        recent = recent.mid(0, 10);
    }
    settings.setValue("recentFiles", recent);
}

QString FileManager::getDefaultSaveLocation() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/CommLink";
    QDir().mkpath(path);
    return path;
}
