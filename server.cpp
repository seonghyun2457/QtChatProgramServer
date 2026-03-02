#include "server.h"

#include <QNetworkInterface>
#include <QThread>
#include <QDebug>

Server::Server(QObject *parent)
    : QTcpServer{parent}
{
    /*
    int idleThreadCount = QThread::idealThreadCount();
    mPool.setMaxThreadCount(idleThreadCount);
    qInfo () << "Max thread count: " << mPool.maxThreadCount();
*/
}

Server::~Server()
{
    quit();
}

void Server::start(quint16 iPort)
{
    QHostAddress connectingAddress;

    QList<QHostAddress> hostAddresses = QNetworkInterface::allAddresses();
    for (size_t i = 0; i < hostAddresses.size(); ++i) {
        if (hostAddresses[i].protocol() == QAbstractSocket::IPv4Protocol && !hostAddresses[i].isLoopback()) {
            connectingAddress = hostAddresses[i];
            break;
        }
    }

    if (connectingAddress.isNull()) {
        qCritical() << "Couldn't find IPv4 address";
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
        QObject::connect(pClientThread->getClient(), &Client::transferReceivedMessage, aClientThread->getClient(), &Client::writeToHost, Qt::QueuedConnection);
        QObject::connect(aClientThread->getClient(), &Client::transferReceivedMessage, pClientThread->getClient(), &Client::writeToHost, Qt::QueuedConnection);
    }

    // Run client thread
    pClientThread->start();

    // Move new client thread to thread list
    mClientThreads.push_back(std::move(pClientThread));
}
