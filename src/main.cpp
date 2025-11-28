#include "commlink/ui/gui.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    CommLinkGUI mainWindow;
    mainWindow.show();
    
    return app.exec();
}
