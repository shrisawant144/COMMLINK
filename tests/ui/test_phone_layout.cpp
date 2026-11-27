#include "commlink/ui/mainwindow.h"
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QDialog>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>
#include <cstdio>

int main(int argc, char** argv) {
    QTemporaryDir state;
    qputenv("XDG_CONFIG_HOME", state.path().toUtf8());
    qputenv("XDG_DATA_HOME", state.path().toUtf8());
    QApplication app(argc, argv);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, state.path());
    QStandardPaths::setTestModeEnabled(true);
    app.setOrganizationName("CommLinkPhoneTest");
    app.setApplicationName("CommLinkPhoneTest");
    MainWindow window;
    window.show();
    auto* tabs = window.findChild<QTabWidget*>("phoneTabs");
    if (!tabs || tabs->count() != 3)
        return 1;
    bool passed = true;
    for (auto theme : {ThemeManager::Light, ThemeManager::Dark}) {
        ThemeManager::instance().setTheme(theme);
        for (const auto size : {QSize(360, 740), QSize(390, 780), QSize(430, 900), QSize(740, 360)}) {
            window.resize(size);
            for (int page = 0; page < tabs->count(); ++page) {
                tabs->setCurrentIndex(page);
                for (int i = 0; i < 8; ++i)
                    app.processEvents();
                if (window.size() != size) {
                    std::fprintf(stderr, "Window grew from %dx%d to %dx%d, page %d\n", size.width(), size.height(), window.width(), window.height(), page);
                    passed = false;
                }
                auto* scroll = qobject_cast<QScrollArea*>(tabs->widget(page));
                if (scroll && scroll->horizontalScrollBar()->maximum() > 0) {
                    std::fprintf(stderr, "Page %d needs horizontal scrolling at %dx%d: %d pixels\n", page, size.width(), size.height(), scroll->horizontalScrollBar()->maximum());
                    passed = false;
                }
                if (theme == ThemeManager::Light && size == QSize(390, 780)) {
                    const QString path = qEnvironmentVariable("COMMLINK_PHONE_SCREENSHOTS");
                    if (!path.isEmpty()) {
                        QDir().mkpath(path);
                        window.grab().save(path + QString("/phone-%1.png").arg(page));
                    }
                }
            }
        }
    }
    window.resize(390, 780);
    tabs->setCurrentIndex(1);
    for (const QString title : {QStringLiteral("Options"), QStringLiteral("Replay")}) {
        QPushButton* action = nullptr;
        for (auto* button : window.findChildren<QPushButton*>())
            if (button->text() == title)
                action = button;
        if (!action) {
            passed = false;
            continue;
        }
        bool checked = false;
        QTimer::singleShot(0, [&] {
            auto* dialog = qobject_cast<QDialog*>(app.activeModalWidget());
            if (dialog) {
                if (dialog->width() > window.width() || dialog->height() > window.height()) {
                    std::fprintf(stderr, "%s dialog does not fit phone: %dx%d\n", title.toUtf8().constData(), dialog->width(), dialog->height());
                    passed = false;
                }
                checked = true;
                dialog->reject();
            }
        });
        action->click();
        passed &= checked;
    }
    std::fprintf(stderr, "Phone layout checks: %s\n", passed ? "PASS" : "FAIL");
    return passed ? 0 : 1;
}
