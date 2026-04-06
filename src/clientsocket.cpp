#include "clientsocket.h"

#include <QThread>
#include <QAbstractSocket>
#include <QMetaEnum>

ClientSocket::ClientSocket(QObject *parent)
    : QTcpSocket{parent}
    , mHeartBeatTimer(nullptr)
    , mHeartBeatCount(0)
    , mHEARTBEAT_INTERVAL_SECOND(10 * 1000)
    , mExpectedMessageSize(0)
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

void ClientSocket::send(const QByteArray& iMessage)
{
    qDebug() << "iMessage: " << iMessage;

    writePacket(iMessage);
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
        // If we don't know message size
        if (mExpectedMessageSize == 0) {
            if (mBuffer.size() < sizeof(quint32)) {
                break;
            }

            // Read header (4 Bytes)
            QDataStream stream(mBuffer);
            stream >> mExpectedMessageSize;
            qDebug() << "mBuffer: " << mBuffer;

            // Delete header from buffer
            mBuffer.remove(0, sizeof(quint32));
        }

        // We know message size but the whole message isn't given yet
        if (mBuffer.size() < mExpectedMessageSize) {
            break;
        }

        // Whole message is given    
        QByteArray messageData = mBuffer.sliced(0, mExpectedMessageSize);
        mBuffer.remove(0, mExpectedMessageSize);
        mExpectedMessageSize = 0;

        QString message = QString::fromUtf8(messageData);
        qDebug() << "Parsed complete message from " << peerAddress().toString() << ":" << message;

        // Heartbeat
        if (message == "Client: heartbeat pong\n") {
            continue;
        }

        emit broadcast(messageData);
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
    writePacket("Server: heartbeat ping\n");

    mHeartBeatCount++;
}

void ClientSocket::writePacket(const QByteArray &iMessage)
{
    // packet = header(4bytes) + message
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << static_cast<quint32>(iMessage.size());
    packet.append(iMessage);
    write(packet);
}

