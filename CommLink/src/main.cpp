#include <QApplication>
#include "../include/gui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(true);
    
    CommLinkGUI window;
    window.show();
    
    return app.exec();
}