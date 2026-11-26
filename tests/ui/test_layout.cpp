#include "commlink/ui/historytab.h"
#include "commlink/ui/mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QGroupBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionGroupBox>
#include <QTabBar>
#include <QTemporaryDir>

namespace {
void settleLayout() {
    for (int i = 0; i < 5; ++i) {
        QApplication::processEvents();
    }
}

bool check(bool condition, const QString& message) {
    if (!condition) {
        qCritical().noquote() << message;
    }
    return condition;
}

bool checkChildren(QWidget* container) {
    bool passed = true;
    const auto children = container->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (int i = 0; i < children.size(); ++i) {
        auto* child = children[i];
        if (!child->isVisible()) {
            continue;
        }
        passed &=
            check(container->rect().contains(child->geometry()),
                  QString("%1 extends outside %2")
                      .arg(child->metaObject()->className(), container->metaObject()->className()));
        for (int j = i + 1; j < children.size(); ++j) {
            if (children[j]->isVisible()) {
                passed &= check(!child->geometry().intersects(children[j]->geometry()),
                                QString("Overlapping controls in %1")
                                    .arg(container->metaObject()->className()));
            }
        }
    }
    return passed;
}

bool checkGroups(MainWindow& window) {
    bool passed = true;
    for (auto* group : window.findChildren<QGroupBox*>()) {
        QStyleOptionGroupBox option;
        option.initFrom(group);
        option.text = group->title();
        option.textAlignment = group->alignment();
        const QRect title = group->style()->subControlRect(QStyle::CC_GroupBox, &option,
                                                           QStyle::SC_GroupBoxLabel, group);

        passed &= check(group->height() >= group->minimumSizeHint().height(),
                        QString("Compressed panel: %1").arg(group->title()));
        passed &= checkChildren(group);
        const auto children = group->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
        for (auto* child : children) {
            if (child->isVisible()) {
                passed &= check(child->y() > title.bottom(),
                                QString("Control overlaps header: %1").arg(group->title()));
            }
        }
    }
    return passed;
}
} // namespace

int main(int argc, char** argv) {
    QTemporaryDir state;
    if (!state.isValid()) {
        return 1;
    }
    // Exercise real settings and history without changing the user's data.
    qputenv("XDG_CONFIG_HOME", state.path().toUtf8());
    qputenv("XDG_DATA_HOME", state.path().toUtf8());
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, state.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, state.path());
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QApplication app(argc, argv);
    app.setApplicationName("CommLinkLayoutTest");
    MainWindow window;
    window.show();
    settleLayout();

    auto* controls = window.findChild<QScrollArea*>("controlsScrollArea");
    auto* connection = window.findChild<ConnectionPanel*>();
    auto* tabs = window.findChild<QTabWidget*>();
    auto* history = window.findChild<HistoryTab*>();
    bool passed = check(controls && connection && tabs && history, "Missing application panels");
    if (!passed) {
        return 1;
    }

    for (auto theme : {ThemeManager::Light, ThemeManager::Dark}) {
        ThemeManager::instance().setTheme(theme);
        for (const auto& size : {QSize(1400, 800), QSize(1000, 600)}) {
            window.resize(size);
            settleLayout();
            passed &= check(window.size() == size, "Window cannot fit the supported size");
            passed &= check(window.minimumSizeHint().height() <= size.height(),
                            "Panels require more height than the window provides");

            for (const auto& protocol : {"TCP", "HTTP", "WebSocket", "UDP"}) {
                connection->setProtocol(protocol);
                settleLayout();
                for (auto* label : connection->findChildren<QLabel*>()) {
                    if (label->text() == "HTTP Method:") {
                        passed &= check(label->isVisible() == (connection->getProtocol() == "HTTP"),
                                        "HTTP method label does not match the selected protocol");
                    } else if (label->text() == "Port:") {
                        const bool needsPort = connection->getProtocol() == "TCP" ||
                                               connection->getProtocol() == "UDP";
                        passed &= check(label->isVisible() == needsPort,
                                        "Port label does not match the selected protocol");
                    }
                }

                for (int tab = 0; tab < tabs->count(); ++tab) {
                    tabs->setCurrentIndex(tab);
                    settleLayout();
                    passed &= checkGroups(window);
                    passed &= checkChildren(history);
                    passed &= check(tabs->tabBar()->rect().contains(tabs->tabBar()->tabRect(tab)),
                                    "Selected tab is clipped after switching views");
                }
            }

            QPushButton* send = nullptr;
            // Find the primary action by its accessible name, independent of button order.
            for (auto* button : window.findChild<MessagePanel*>()->findChildren<QPushButton*>()) {
                if (button->accessibleName() == "Send Message") {
                    send = button;
                }
            }
            if (!check(send != nullptr, "Missing Send Message action")) {
                return 1;
            }
            controls->ensureWidgetVisible(send);
            settleLayout();
            const QRect sendRect(send->mapTo(controls->viewport(), QPoint()), send->size());
            passed &= check(controls->viewport()->rect().contains(sendRect),
                            "Send Message cannot be reached by scrolling the control column");
        }
    }

    if (passed) {
        qInfo() << "Layout checks passed across both themes, supported sizes, protocols and tabs.";
    }
    return passed ? 0 : 1;
}
