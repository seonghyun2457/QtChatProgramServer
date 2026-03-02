#include "client.h"

#include <QThread>
#include <QAbstractSocket>
#include <QMetaEnum>

Client::Client(qintptr socketDescriptor, QObject *parent)
    : QObject{parent}
    , mpSocket(nullptr)
    , mSocketDescriptor(socketDescriptor)
{

}

void Client::connected()
{
    qDebug() << "Client connected event";
}

void Client::disconnected()
{
    qDebug() << "Client disconnected";
    mpSocket->close();
    qDebug() << "Socket closed";

    QThread::currentThread()->quit();
}

void Client::error(QAbstractSocket::SocketError socketError)
{
    qCritical() << "Error: " << socketError << " " << mpSocket->errorString();
}

void Client::stateChanged(QAbstractSocket::SocketState socketState)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
    qDebug() << "State: " << metaEnum.valueToKey(socketState);
}

void Client::readyRead()
{
    qDebug() << "Data from: " << sender() << " bytes: " << mpSocket->bytesAvailable();
    QByteArray rawReadData = mpSocket->readAll();

    emit transferReceivedMessage(rawReadData);
}

void Client::writeToHost(const QByteArray& iMessage)
{
    qDebug() << "iMessage: " << iMessage;
    mpSocket->write(iMessage);
}

bool Client::openTcpSocket()
{
    mpSocket = std::make_unique<QTcpSocket>();

    if (mpSocket == nullptr) {
        qCritical() << "Client TCP socket is null!";
        return false;
    }

    // CONNECT
    connect(mpSocket.get(), &QTcpSocket::connected, this, &Client::connected);
    connect(mpSocket.get(), &QTcpSocket::disconnected, this, &Client::disconnected);

    connect(mpSocket.get(), &QTcpSocket::stateChanged, this, &Client::stateChanged);
    connect(mpSocket.get(), &QTcpSocket::readyRead, this, &Client::readyRead);

    connect(mpSocket.get(), &QTcpSocket::errorOccurred, this, &Client::error);

    // Set Socket Descriptor
    if (!mpSocket->setSocketDescriptor(mSocketDescriptor)) {
        qCritical() << mpSocket->errorString();
        return false;
    }

    qDebug() << "TCP socket opens on a thread:" << QThread::currentThread();
    qDebug() << "Port Num:" << mpSocket->localPort();
    qDebug() << "Address:" << mpSocket->localAddress();

    return true;
}

void Client::run()
{
    QThread* pCurrentThread = QThread::currentThread();
    qDebug() << "Client thread: " << pCurrentThread;

    if (!openTcpSocket()) {
        qCritical() << "Failed to open Cleint TCP socket!";
        return;
    }

}
