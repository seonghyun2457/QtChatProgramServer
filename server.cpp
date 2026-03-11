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

const QList<QHostAddress> Server::getIPv4Addresses() const
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
    // Disconnect clients
    for (ClientThread* aClientThread : mClientThreads) {
        ClientSocket* pSocket = aClientThread->getClientSocket();

        // Disconnect all signals
        pSocket->disconnect();

        // Abort socket
        QMetaObject::invokeMethod(pSocket, [pSocket]() { pSocket->abort(); }, Qt::BlockingQueuedConnection);

        // Delete Client Thread when thread is termianted
        QObject::connect(aClientThread, &QThread::finished, aClientThread, &QObject::deleteLater);

        // Terminate thread
        aClientThread->quit();

        // Wait until thread is terminated
        aClientThread->wait();

        qDebug() << "A client disconnected";
    }

    // clear thread list
    mClientThreads.clear();

    // Close server
    close();
    qDebug() << "Server closed";
}

void Server::disconnectClient()
{
    ClientSocket* pClientSocket = static_cast<ClientSocket*>(sender());

    auto clientThreadIt = mClientThreads.begin();
    while (clientThreadIt != mClientThreads.end()) {
        if ((*clientThreadIt)->getClientSocket() == pClientSocket) {

            ClientThread* pClientThread = *clientThreadIt;
            qDebug() << "Client socket disconnected. Removing from list.";

            mClientThreads.erase(clientThreadIt);

            // Delete Client Thread when thread is termianted
            QObject::connect(pClientThread, &QThread::finished, pClientThread, &QObject::deleteLater);

            // Terminate thread
            pClientThread->quit();
            break;
        }
        else {
            ++clientThreadIt;
        }
    }
}

void Server::broadcast(const QByteArray &iMessage)
{
    qDebug() << "Broadcasting message to all clients:" << iMessage;

    ClientSocket* pClientSocket = static_cast<ClientSocket*>(sender());

    for (const auto& clientThread : mClientThreads) {
        if (clientThread->getClientSocket() != pClientSocket) {

            // call send method in different thread
            QMetaObject::invokeMethod(clientThread->getClientSocket(), "send", Qt::QueuedConnection, Q_ARG(QByteArray, iMessage));
        }
    }
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    qInfo() << "Incomming connection " << socketDescriptor << " on " << QThread::currentThread();

    // Create a new client thread
    ClientThread* pClientThread = new ClientThread(socketDescriptor);

    // CONNECT
    QObject::connect(pClientThread->getClientSocket(), &QTcpSocket::disconnected, this, &Server::disconnectClient);
    QObject::connect(pClientThread->getClientSocket(), &ClientSocket::broadcast, this, &Server::broadcast, Qt::QueuedConnection);

    // Run client thread
    pClientThread->start();

    // Move new client thread to thread list
    mClientThreads.push_back(pClientThread);
}

