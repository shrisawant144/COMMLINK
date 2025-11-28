#include "commlink/logger.h"
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QTextDocument>
#include <QtCore/QDateTime>
#include <QtGui/QTextCursor>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QScrollBar>

LoggerWidget::LoggerWidget(QWidget *parent)
    : QWidget(parent), maxLogLines(500)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    logDisplay = new QTextEdit();
    logDisplay->setReadOnly(true);
    logDisplay->setStyleSheet(
        "QTextEdit {"
        "    background-color: #1a1a1a;"
        "    color: #e0e0e0;"
        "    border: 1px solid #333333;"
        "    border-radius: 4px;"
        "    font-family: 'Courier New', monospace;"
        "    font-size: 10px;"
        "    padding: 8px;"
        "    margin: 0px;"
        "}"
        "QTextEdit:focus {"
        "    border: 1px solid #00A99D;"
        "}"
        "QScrollBar:vertical {"
        "    background-color: #2a2a2a;"
        "    width: 12px;"
        "    border: 1px solid #333333;"
        "    border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: #00A99D;"
        "    border-radius: 6px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: #00C9B7;"
        "}"
        "QScrollBar:horizontal {"
        "    background-color: #2a2a2a;"
        "    height: 12px;"
        "    border: 1px solid #333333;"
        "    border-radius: 6px;"
        "}"
    );
    
    layout->addWidget(logDisplay);
}

LoggerWidget::~LoggerWidget() {}

void LoggerWidget::logInfo(const QString &message) {
    log(message, LogLevel::Info);
}

void LoggerWidget::logWarning(const QString &message) {
    log(message, LogLevel::Warning);
}

void LoggerWidget::logError(const QString &message) {
    log(message, LogLevel::Error);
}

void LoggerWidget::logDebug(const QString &message) {
    log(message, LogLevel::Debug);
}

void LoggerWidget::logSuccess(const QString &message) {
    log(message, LogLevel::Success);
}

void LoggerWidget::clear() {
    logDisplay->clear();
}

void LoggerWidget::setMaxLines(int maxLines) {
    maxLogLines = maxLines;
}

QString LoggerWidget::getLevelIcon(LogLevel level) const {
    switch (level) {
    case LogLevel::Info:
        return "ℹ";
    case LogLevel::Warning:
        return "⚠";
    case LogLevel::Error:
        return "✘";
    case LogLevel::Debug:
        return "🐛";
    case LogLevel::Success:
        return "✓";
    }
    return "•";
}

QString LoggerWidget::getLevelColor(LogLevel level) const {
    switch (level) {
    case LogLevel::Info:
        return "#00A99D"; // CommLink teal
    case LogLevel::Warning:
        return "#FFA500"; // Orange
    case LogLevel::Error:
        return "#FF6B6B"; // Red
    case LogLevel::Debug:
        return "#9B59B6"; // Purple
    case LogLevel::Success:
        return "#2ECC71"; // Green
    }
    return "#e0e0e0";
}

QString LoggerWidget::getLevelName(LogLevel level) const {
    switch (level) {
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Success:
        return "SUCCESS";
    }
    return "LOG";
}

void LoggerWidget::log(const QString &message, LogLevel level) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString levelName = getLevelName(level);
    QString color = getLevelColor(level);
    QString icon = getLevelIcon(level);
    
    // Format: [HH:MM:SS.ZZZ] [LEVEL] message
    QString logEntry = QString(
        "<span style='color: %1;'>[%2]</span> "
        "<span style='color: %3; font-weight: bold;'>%4 %5</span> "
        "<span style='color: #b0b0b0;'>%6</span>"
    )
    .arg(color)
    .arg(timestamp)
    .arg(color)
    .arg(icon)
    .arg(levelName)
    .arg(message);
    
    // Append to log
    logDisplay->append(logEntry);
    
    // Auto-scroll to bottom
    QTextCursor cursor = logDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    logDisplay->setTextCursor(cursor);
    
    // Trim old logs if exceeding max lines
    trimOldLogs();
}

void LoggerWidget::trimOldLogs() {
    QTextDocument *doc = logDisplay->document();
    int lineCount = doc->blockCount();
    
    if (lineCount > maxLogLines) {
        QTextCursor cursor(doc);
        cursor.movePosition(QTextCursor::Start);
        
        // Remove lines from the beginning
        int linesToRemove = lineCount - maxLogLines;
        for (int i = 0; i < linesToRemove; ++i) {
            cursor.select(QTextCursor::BlockUnderCursor);
            cursor.removeSelectedText();
            cursor.deleteChar(); // Remove the newline
        }
    }
}
