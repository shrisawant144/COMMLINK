#include "commlink/ui/gui.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    GUI mainWindow;
    mainWindow.show();
    
    return app.exec();
}
