#include <QCoreApplication>

#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server chatServer;
    const quint16 IP_PORT = 25000;

    chatServer.start(IP_PORT);


    return a.exec();
}
