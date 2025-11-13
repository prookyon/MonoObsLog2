#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application information
    QApplication::setApplicationName("MonoObsLog");
    QApplication::setApplicationVersion("0.1.0");
    QApplication::setOrganizationName("Prookyon");

    MainWindow window;
    window.show();

    return app.exec();
}