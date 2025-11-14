#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application information
    QApplication::setApplicationName("MonoObsLog");
    QApplication::setApplicationVersion("0.2.0");
    QApplication::setOrganizationName("Prookyon");
    QApplication::setWindowIcon(QIcon(":/images/icon.ico"));

    MainWindow window;
    window.show();

    return app.exec();
}