#include "clientthread.h"

ClientThread::ClientThread(qintptr socketDescriptor, QObject *parent)
    : QThread{parent}
    , mClientSocket(new ClientSocket())
{
    setObjectName("Client thread");

    mClientSocket->moveToThread(this);

    QObject::connect(this, &QThread::started, mClientSocket, [this, socketDescriptor](){ mClientSocket->initSocket(socketDescriptor); });
    connect(this, &QThread::finished, mClientSocket, &QObject::deleteLater);
}

ClientSocket* ClientThread::getClientSocket() const
{
    return mClientSocket;
}
