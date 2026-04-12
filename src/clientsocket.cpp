#include "clientsocket.h"

#include <QThread>
#include <QAbstractSocket>
#include <QMetaEnum>

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
    while (true) {
        // Read header
        PacketHeader header;
        const char* pData = mBuffer.data();
        memcpy(&header, pData, sizeof(PacketHeader));

        // If we don't know message size
        if (header.packetSize == 0) {
            if (mBuffer.size() < sizeof(PacketHeader)) {
                break;
            }
        }

        // We know message size but the whole message isn't given yet
        if (mBuffer.size() < header.packetSize + sizeof(PacketHeader)) {
            break;
        }

        // Whole message is given
        QByteArray broadcastingData(mBuffer);
        mBuffer.clear();

        qDebug() << "Parsed complete message from " << peerAddress().toString() << ":" << broadcastingData;

        // Heartbeat
        if (header.packetType == ePacketType::Heartbeat) {
            break;
        }

        emit broadcast(broadcastingData);
    }
}

void ClientSocket::sendHeartBeat()
{
    if (mHeartBeatCount >= 3) {
        qWarning() << "Heartbeat timeout. No response for 3 minutes from " << peerAddress() << ". Disconnecting...";
        abort();
        return;
    }

    qDebug() << "Sending heartbeat (ping) to client. Miss count: " << mHeartBeatCount;
    writePacket(ePacketType::Heartbeat, "Server: heartbeat ping\n");

    mHeartBeatCount++;
}

void ClientSocket::writePacket(const ePacketType iPacketType, const QByteArray &iPayload)
{
    PacketHeader packetHeader;
    packetHeader.packetType = iPacketType;
    packetHeader.packetSize = iPayload.size();

    switch (iPacketType) {
    case ePacketType::Heartbeat:
        break;
    case ePacketType::TextMessage:
        break;
    case ePacketType::File:
        break;
    default:
        qCritical() << "Invalid packet type.";
        return;
    }

    // packet = header + payload
    QByteArray packet;
    packet.reserve(sizeof(PacketHeader) + packetHeader.packetSize);

    packet.append((char*)&packetHeader, sizeof(PacketHeader));
    packet.append(iPayload);

    write(packet);
}

