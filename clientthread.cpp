#include "clientthread.h"

ClientThread::ClientThread(qintptr socketDescriptor, QObject *parent)
    : QThread{parent}
    , mClient(std::make_unique<Client>(socketDescriptor))
{
    if (mClient == nullptr) {
        return;
    }

    setObjectName("Client thread");

    QObject::connect(this, &QThread::started, mClient.get(), &Client::run);
    mClient->moveToThread(this);
}

Client* ClientThread::getClient() const
{
    return mClient.get();
}
