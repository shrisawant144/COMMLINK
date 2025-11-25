#include "../include/thememanager.h"
#include <QTimer>
#include <QStyleFactory>
#include <QWidget>

ThemeManager::ThemeManager() {
    settings = new QSettings("CommLink", "CommLinkApp", this);
    
    // Monitor system theme changes
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ThemeManager::onSystemThemeChanged);
    timer->start(5000); // Check every 5 seconds
}

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

void ThemeManager::setTheme(Theme theme) {
    if (currentThemeMode != theme) {
        currentThemeMode = theme;
        saveSettings();
        emit themeChanged();
    }
}

bool ThemeManager::isDarkMode() const {
    switch (currentThemeMode) {
        case Dark: return true;
        case Light: return false;
        case Auto: return isSystemDark();
    }
    return false;
}

bool ThemeManager::isSystemDark() const {
    QPalette palette = QApplication::palette();
    return palette.color(QPalette::Window).lightness() < 128;
}

QString ThemeManager::getStyleSheet() const {
    return isDarkMode() ? getDarkStyleSheet() : getLightStyleSheet();
}

QString ThemeManager::getLightStyleSheet() const {
    return R"(
        QWidget {
            background-color: #ffffff;
            color: #000000;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 1ex;
            background-color: #f9f9f9;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 10px 0 10px;
        }
        QPushButton {
            padding: 8px 16px;
            border-radius: 4px;
            background-color: #f0f0f0;
            border: 1px solid #cccccc;
            color: #000000;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
        QPushButton:pressed {
            background-color: #d0d0d0;
        }
        QPushButton:disabled {
            background-color: #f5f5f5;
            color: #999999;
        }
        QLineEdit, QTextEdit, QComboBox {
            padding: 4px;
            border: 1px solid #cccccc;
            border-radius: 3px;
            background-color: #ffffff;
            color: #000000;
        }
        QLineEdit:focus, QTextEdit:focus {
            border: 2px solid #0078d4;
        }
        QTabWidget::pane {
            border: 1px solid #cccccc;
            background-color: #ffffff;
        }
        QTabBar::tab {
            background-color: #f0f0f0;
            padding: 8px 16px;
            margin-right: 2px;
            border: 1px solid #cccccc;
            border-bottom: none;
        }
        QTabBar::tab:selected {
            background-color: #ffffff;
            border-bottom: 1px solid #ffffff;
        }
        QTabBar::tab:hover {
            background-color: #e0e0e0;
        }
        QStatusBar {
            background-color: #f0f0f0;
            border-top: 1px solid #cccccc;
            color: #000000;
        }
        QMenuBar {
            background-color: #f0f0f0;
            color: #000000;
        }
        QMenuBar::item {
            padding: 4px 8px;
        }
        QMenuBar::item:selected {
            background-color: #e0e0e0;
        }
        QMenu {
            background-color: #ffffff;
            border: 1px solid #cccccc;
        }
        QMenu::item {
            padding: 4px 20px;
        }
        QMenu::item:selected {
            background-color: #e0e0e0;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 4px solid #666666;
        }
    )";
}

QString ThemeManager::getDarkStyleSheet() const {
    return R"(
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #555555;
            border-radius: 5px;
            margin-top: 1ex;
            background-color: #3c3c3c;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 10px 0 10px;
            color: #ffffff;
        }
        QPushButton {
            padding: 8px 16px;
            border-radius: 4px;
            background-color: #404040;
            border: 1px solid #555555;
            color: #ffffff;
        }
        QPushButton:hover {
            background-color: #505050;
        }
        QPushButton:pressed {
            background-color: #606060;
        }
        QPushButton:disabled {
            background-color: #333333;
            color: #666666;
        }
        QLineEdit, QTextEdit, QComboBox {
            padding: 4px;
            border: 1px solid #555555;
            border-radius: 3px;
            background-color: #404040;
            color: #ffffff;
        }
        QLineEdit:focus, QTextEdit:focus {
            border: 2px solid #0078d4;
        }
        QTabWidget::pane {
            border: 1px solid #555555;
            background-color: #3c3c3c;
        }
        QTabBar::tab {
            background-color: #404040;
            padding: 8px 16px;
            margin-right: 2px;
            border: 1px solid #555555;
            border-bottom: none;
            color: #ffffff;
        }
        QTabBar::tab:selected {
            background-color: #505050;
            border-bottom: 1px solid #505050;
        }
        QTabBar::tab:hover {
            background-color: #4a4a4a;
        }
        QStatusBar {
            background-color: #404040;
            border-top: 1px solid #555555;
            color: #ffffff;
        }
        QMenuBar {
            background-color: #404040;
            color: #ffffff;
        }
        QMenuBar::item {
            padding: 4px 8px;
        }
        QMenuBar::item:selected {
            background-color: #505050;
        }
        QMenu {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            color: #ffffff;
        }
        QMenu::item {
            padding: 4px 20px;
        }
        QMenu::item:selected {
            background-color: #505050;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 4px solid #cccccc;
        }
        QScrollBar:vertical {
            background-color: #404040;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: #606060;
            border-radius: 6px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #707070;
        }
        QScrollBar:horizontal {
            background-color: #404040;
            height: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:horizontal {
            background-color: #606060;
            border-radius: 6px;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: #707070;
        }
    )";
}

void ThemeManager::loadSettings() {
    int themeValue = settings->value("theme", static_cast<int>(Light)).toInt();
    currentThemeMode = static_cast<Theme>(themeValue);
}

void ThemeManager::saveSettings() {
    settings->setValue("theme", static_cast<int>(currentThemeMode));
}

void ThemeManager::applyTheme(QWidget* widget) {
    if (widget) {
        widget->setStyleSheet(getStyleSheet());
    }
}

QString ThemeManager::getThemeName() const {
    switch (currentThemeMode) {
        case Light: return "Light";
        case Dark: return "Dark";
        case Auto: return "Auto";
    }
    return "Light";
}

void ThemeManager::onSystemThemeChanged() {
    if (currentThemeMode == Auto) {
        emit themeChanged();
    }
}

#include "thememanager.moc"