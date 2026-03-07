#include "server.h"

#include <QNetworkInterface>
#include <QThread>
#include <QDebug>

Server::Server(QObject *parent)
    : QTcpServer{parent}
{

}

Server::~Server()
{
    if (isListening()) {
        quit();
    }
}

QList<QHostAddress> Server::getIPv4Addresses() const
{
    QList<QHostAddress> ipv4Addresses;

    QList<QHostAddress> hostAddresses = QNetworkInterface::allAddresses();
    for (QHostAddress& address : hostAddresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback()) {
            ipv4Addresses.push_back(address);
        }
    }

    return ipv4Addresses;
}

void Server::start(const QString &iAddress, const quint16 iPort)
{
    QHostAddress connectingAddress(iAddress);
    if (connectingAddress.isNull()) {
        qCritical() << iAddress << " isn't valid.";
        return;
    }

    if (!listen(connectingAddress, iPort)) {
        qCritical() << errorString();
        return;
    }

    qInfo() << "Server started on IPv4:" << connectingAddress << ", Port number: " << iPort;
}

void Server::quit()
{
    close();
    qInfo() << "Server closed";
}

void Server::disconnectClient()
{
    QObject* signalSender = sender();
    qDebug() << "signal sender: " << signalSender;

    ClientThread* pClientThread = static_cast<ClientThread*>(signalSender);
    auto clientThreadIt = mClientThreads.begin();
    while (clientThreadIt != mClientThreads.end()) {
        if (clientThreadIt->get() == pClientThread) {
            qDebug() << "signal sender: " << signalSender << " removed.";
            mClientThreads.erase(clientThreadIt++);
        }
        else {
            ++clientThreadIt;
        }
    }

}

void Server::incomingConnection(qintptr socketDescriptor)
{
    qInfo() << "Incomming connection " << socketDescriptor << " on " << QThread::currentThread();
    connectClient(socketDescriptor);
}

void Server::connectClient(const qintptr iSocketDescriptor)
{
    // Create a new client thread
    std::unique_ptr<ClientThread> pClientThread = std::make_unique<ClientThread>(iSocketDescriptor);

    // CONNECT
    QObject::connect(pClientThread.get(), &QThread::finished, this, &Server::disconnectClient);
    for (std::unique_ptr<ClientThread>& aClientThread : mClientThreads) {
        QObject::connect(pClientThread->getClient(), &Client::transferReceivedMessage, aClientThread->getClient(), &Client::write, Qt::QueuedConnection);
        QObject::connect(aClientThread->getClient(), &Client::transferReceivedMessage, pClientThread->getClient(), &Client::write, Qt::QueuedConnection);
    }

    // Run client thread
    pClientThread->start();

    // Move new client thread to thread list
    mClientThreads.push_back(std::move(pClientThread));
}
