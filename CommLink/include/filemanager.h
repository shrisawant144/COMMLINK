#pragma once
#include <QString>
#include <QStringList>

class FileManager {
public:
    static QString loadJsonFromFile(const QString& filePath);
    static bool saveJsonToFile(const QString& content, const QString& filePath);
    static QStringList getRecentFiles();
    static void addToRecentFiles(const QString& filePath);
    static QString getDefaultSaveLocation();
};
