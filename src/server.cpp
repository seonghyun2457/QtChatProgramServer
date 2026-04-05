#include "server.h"

#include <QNetworkInterface>
//#include <QThread>
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
    for (ClientSocket* pClient : mClients) {
        pClient->disconnect();
        pClient->abort();
        pClient->deleteLater();
    }

    // clear thread list
    mClients.clear();

    // Close server
    close();
    qDebug() << "Server closed";
}

void Server::disconnectClient()
{
    ClientSocket* pSenderClient = static_cast<ClientSocket*>(sender());

    if (pSenderClient) {
        mClients.remove(pSenderClient);
        pSenderClient->deleteLater();
        qDebug() << "Client socket disconnected and removed from list. Total clients:" << mClients.size();
    }
}

void Server::broadcast(const QByteArray &iMessage)
{
    qDebug() << "Broadcasting message to all clients:" << iMessage;

    ClientSocket* pSenderClient = static_cast<ClientSocket*>(sender());

    for (ClientSocket* pClient : mClients) {
        if (pSenderClient != pClient) {
            pClient->send(iMessage);
        }
    }
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    qInfo() << "Incomming connection " << socketDescriptor << " on " << QThread::currentThread();

    // Create a new client socket
    ClientSocket* pClientSocket = new ClientSocket(this);

    // CONNECT
    QObject::connect(pClientSocket, &QTcpSocket::disconnected, this, &Server::disconnectClient);
    QObject::connect(pClientSocket, &ClientSocket::broadcast, this, &Server::broadcast);

    // Initialize socket
    pClientSocket->initSocket(socketDescriptor);

    // Push a client to clients list
    mClients.push_back(pClientSocket);
}

