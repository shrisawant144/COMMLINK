#ifndef LOGGER_H
#define LOGGER_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTextEdit>
#include <QtCore/QString>
#include <QtCore/QDateTime>

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug,
    Success
};

class LoggerWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoggerWidget(QWidget *parent = nullptr);
    ~LoggerWidget();

    void logInfo(const QString &message);
    void logWarning(const QString &message);
    void logError(const QString &message);
    void logDebug(const QString &message);
    void logSuccess(const QString &message);
    
    void clear();
    void setMaxLines(int maxLines);
    
private:
    QTextEdit *logDisplay;
    int maxLogLines;
    
    void log(const QString &message, LogLevel level);
    QString getLevelIcon(LogLevel level) const;
    QString getLevelColor(LogLevel level) const;
    QString getLevelName(LogLevel level) const;
    void trimOldLogs();
};

#endif // LOGGER_H
