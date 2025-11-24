#include <QApplication>
#include "../include/gui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    JsonSenderGUI window;
    window.show();
    
    return app.exec();
}