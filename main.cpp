#include <QApplication>

#include "serverwindow.h"
#include "server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ServerWindow serverWindow;
    serverWindow.show();
    /*
    Server chatServer;
    const quint16 IP_PORT = 25000;

    chatServer.start(IP_PORT);
    */

    return a.exec();
}
