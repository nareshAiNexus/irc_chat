#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("IRC Client");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("QtIRC");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
