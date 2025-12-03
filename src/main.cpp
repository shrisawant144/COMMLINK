#include "commlink/ui/gui.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    // Enable High DPI scaling for better display on 4K/5K monitors
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QApplication app(argc, argv);
    
    // Set application metadata for better system integration
    QApplication::setApplicationName("CommLink");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("CommLink");
    QApplication::setOrganizationDomain("commlink.app");
    
    CommLinkGUI mainWindow;
    mainWindow.show();
    
    return QApplication::exec();
}
