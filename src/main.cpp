#include "commlink/ui/gui.h"
#include "commlink/ui/mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

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
    
    // Command line option to choose GUI version
    QCommandLineParser parser;
    parser.setApplicationDescription("CommLink - Network Communication Tool");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption legacyOption(QStringList() << "l" << "legacy",
        "Use legacy monolithic GUI (default is new modular GUI)");
    parser.addOption(legacyOption);
    
    parser.process(app);
    
    // Launch appropriate GUI based on command line argument
    if (parser.isSet(legacyOption)) {
        // Use old monolithic GUI
        CommLinkGUI mainWindow;
        mainWindow.show();
        return QApplication::exec();
    } else {
        // Use new modular GUI (default)
        MainWindow mainWindow;
        mainWindow.show();
        return QApplication::exec();
    }
}
