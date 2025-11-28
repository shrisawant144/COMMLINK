#pragma once
#include <QString>
#include <QObject>
#include <QSettings>
#include <QApplication>
#include <QPalette>

class ThemeManager : public QObject {
    Q_OBJECT

public:
    enum Theme { Light, Dark, Auto };
    
    static ThemeManager& instance();
    void setTheme(Theme theme);
    Theme currentTheme() const { return currentThemeMode; }
    bool isDarkMode() const;
    QString getStyleSheet() const;
    void loadSettings();
    void saveSettings();
    void applyTheme(QWidget* widget);
    QString getThemeName() const;
    
signals:
    void themeChanged();
    
private slots:
    void onSystemThemeChanged();
    
private:
    ThemeManager();
    bool isSystemDark() const;
    QString getLightStyleSheet() const;
    QString getDarkStyleSheet() const;
    
    Theme currentThemeMode = Light;
    QSettings* settings;
};