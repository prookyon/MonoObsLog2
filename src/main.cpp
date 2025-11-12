#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application information
    QApplication::setApplicationName("MGW Observation Log");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("MGW");

    MainWindow window;
    window.show();

    return app.exec();
}