#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>
#include <QStringList>
#include "dataformat.h"

class FileManager {
public:
    static QString loadMessageFromFile(const QString& filePath, DataFormatType format);
    static bool saveMessageToFile(const QString& content, const QString& filePath, DataFormatType format);
    static QString getFileExtension(DataFormatType format);
    static QStringList getRecentFiles();
    static void addToRecentFiles(const QString& filePath);
    static QString getDefaultSaveLocation();
};

#endif // FILEMANAGER_H
