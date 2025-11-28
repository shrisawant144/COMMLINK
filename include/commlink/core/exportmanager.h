#pragma once
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QJsonDocument>
#include "dataformat.h"

class ExportManager {
public:
    static bool exportLogs(const QStringList& logs, const QString& format, const QString& filePath);
    static bool exportMessages(const QList<DataMessage>& messages, const QString& format, const QString& filePath);
    static QStringList getSupportedFormats();
};
