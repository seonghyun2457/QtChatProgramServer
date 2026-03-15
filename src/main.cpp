#include <QApplication>

#include "serverwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ServerWindow serverWindow;
    serverWindow.show();

    return a.exec();
}
