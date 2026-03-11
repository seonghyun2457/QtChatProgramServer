#include "clientsocket.h"

#include <QThread>
#include <QAbstractSocket>
#include <QMetaEnum>

ClientSocket::ClientSocket(QObject *parent)
    : QTcpSocket{parent}
    , mHeartBeatTimer(nullptr)
    , mHeartBeatCount(0)
{
    // CONNECT
    connect(this, &QTcpSocket::connected, this, &ClientSocket::connected);
    connect(this, &QTcpSocket::disconnected, this, &ClientSocket::disconnected);
    connect(this, &QTcpSocket::stateChanged, this, &ClientSocket::stateChanged);
    connect(this, &QTcpSocket::readyRead, this, &ClientSocket::readyRead);
    connect(this, &QTcpSocket::errorOccurred, this, &ClientSocket::error);
}

ClientSocket::~ClientSocket()
{
    delete mHeartBeatTimer;
    mHeartBeatTimer = nullptr;
}


bool ClientSocket::isConnected() const
{
    return state() == QAbstractSocket::SocketState::ConnectedState;
}

void ClientSocket::initSocket(const qintptr socketDescriptor)
{
    // Set Socket Descriptor
    if (!setSocketDescriptor(socketDescriptor)) {
        qCritical() << errorString();
        return;
    }

    if (state() != QAbstractSocket::SocketState::ConnectedState) {
        qCritical() << "Socket isn't connected. Current State: " << state();
        return;
    }


    // Set HeartBeat
    mHeartBeatCount = 0;
    mHeartBeatTimer = new QTimer();
    QObject::connect(mHeartBeatTimer, &QTimer::timeout, this, &ClientSocket::sendHeartBeat);
    mHeartBeatTimer->start(10 * 1000); // 10 seconds invertal

    qDebug() << "Hearbeat timer started (10 seconds interval)";
    qDebug() << "TCP socket opens on a thread:" << QThread::currentThread();
    qDebug() << "Server Address:" << localAddress();
    qDebug() << "Server Port:" << localPort();
    qDebug() << "Client Address:" << peerAddress();
    qDebug() << "Client Port:" << peerPort();
}

void ClientSocket::send(const QByteArray& iMessage)
{
    qDebug() << "iMessage: " << iMessage;
    write(iMessage);
}

void ClientSocket::connected()
{
    qDebug() << "Client connected event";
}

void ClientSocket::disconnected()
{
    qDebug() << "Client disconnected";
}

void ClientSocket::error(QAbstractSocket::SocketError socketError)
{
    qCritical() << "Error: " << socketError << " " << errorString();
}

void ClientSocket::stateChanged(QAbstractSocket::SocketState socketState)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
    qDebug() << "State: " << metaEnum.valueToKey(socketState);
}

void ClientSocket::readyRead()
{
    // Reset HeartBeat count
    mHeartBeatCount = 0;

    qDebug() << "Data from: " << sender() << " bytes: " << bytesAvailable();
    QByteArray rawReadData = readAll();

    emit broadcast(rawReadData);
}

void ClientSocket::sendHeartBeat()
{
    if (mHeartBeatCount >= 3) {
        qWarning() << "Heartbeat timeout. No response for 3 minutes from " << peerAddress() << ". Disconnecting...";
        abort();
        return;
    }

    qDebug() << "Sending hearbeat (ping) to client. Miss count: " << mHeartBeatCount;
    write("Server: heartbeat ping\n");

    mHeartBeatCount++;
}

