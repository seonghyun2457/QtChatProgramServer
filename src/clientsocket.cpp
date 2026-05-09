#include "clientsocket.h"

#include <QThread>
#include <QAbstractSocket>
#include <QMetaEnum>

const QString ClientSocket::s_heartbeatMessage = "heartbeat ping";

ClientSocket::ClientSocket(QObject *parent)
    : QTcpSocket{parent}
    , mHeartBeatTimer(nullptr)
    , mHeartBeatCount(0)
    , mHEARTBEAT_INTERVAL_SECOND(10 * 1000)
{
    // CONNECT
    connect(this, &QTcpSocket::connected, this, &ClientSocket::connected);
    connect(this, &QTcpSocket::disconnected, this, &ClientSocket::disconnected);
    connect(this, &QTcpSocket::stateChanged, this, &ClientSocket::stateChanged);
    connect(this, &QTcpSocket::readyRead, this, &ClientSocket::readyRead);
    connect(this, &QTcpSocket::errorOccurred, this, &ClientSocket::error);
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
        emit disconnected();
        return;
    }

    if (state() != QAbstractSocket::SocketState::ConnectedState) {
        qCritical() << "Socket isn't connected. Current State: " << state();
        emit disconnected();
        return;
    }

    // Set HeartBeat
    mHeartBeatTimer = new QTimer(this);
    if (mHeartBeatTimer == nullptr) {
        qCritical() << "Timer is null!";
        emit disconnected();
        return;
    }

    QObject::connect(mHeartBeatTimer, &QTimer::timeout, this, &ClientSocket::sendHeartBeat);
    mHeartBeatTimer->start(mHEARTBEAT_INTERVAL_SECOND); // mHEARTBEAT_INTERVAL_SECOND invertal

    qDebug() << "Hearbeat timer started (10 seconds interval)";
    qDebug() << "TCP socket opens on a thread:" << QThread::currentThread();
    qDebug() << "Server Address:" << localAddress();
    qDebug() << "Server Port:" << localPort();
    qDebug() << "Client Address:" << peerAddress();
    qDebug() << "Client Port:" << peerPort();
}

void ClientSocket::send(const QByteArray& iPacket)
{
    qDebug() << "iMessage: " << iPacket;
    write(iPacket);
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
    // Reset HeartBeat
    mHeartBeatCount = 0;
    mHeartBeatTimer->start(mHEARTBEAT_INTERVAL_SECOND); // mHEARTBEAT_INTERVAL_SECOND invertal

    // Push message to buffer
    mBuffer.append(readAll());
    qDebug() << "mBuffer: " << mBuffer;

    // Parse message
    // Read header
    PacketHeader header;
    const char* pData = mBuffer.data();
    memcpy(&header, pData, sizeof(PacketHeader));

    // Whole message is given
    QByteArray broadcastingData(mBuffer);
    mBuffer.remove(0, sizeof(PacketHeader) + header.packetSize);

    qDebug() << "Parsed complete message from " << peerAddress().toString() << ":" << broadcastingData;

    // Heartbeat
    if (header.packetType == ePacketType::Heartbeat) {
        return;
    }

    emit broadcast(broadcastingData);
}

void ClientSocket::sendHeartBeat()
{
    if (mHeartBeatCount >= 3) {
        qWarning() << "Heartbeat timeout. No response for 3 minutes from " << peerAddress() << ". Disconnecting...";
        abort();
        return;
    }

    qDebug() << "Sending heartbeat (ping) to client. Miss count: " << mHeartBeatCount;


    PacketHeader header;
    header.packetType = ePacketType::Heartbeat;
    header.packetSize = s_heartbeatMessage.size();
    qstrncpy(header.senderNickName, "Server", sizeof(header.senderNickName));
    memset(header.fileName, 0, sizeof(header.fileName));
    writePacket(header, s_heartbeatMessage.toUtf8());

    mHeartBeatCount++;
}

void ClientSocket::writePacket(const PacketHeader iPacketHeader, const QByteArray &iPayload)
{
    // packet = header + payload
    QByteArray packet;
    packet.reserve(sizeof(PacketHeader) + iPacketHeader.packetSize);

    packet.append((char*)&iPacketHeader, sizeof(PacketHeader));
    packet.append(iPayload);

    write(packet);
}

