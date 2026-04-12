#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>

#include "packetHeader.h"

class Server;

class ClientSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit ClientSocket(QObject *parent = nullptr);
    virtual ~ClientSocket() = default;

    bool isConnected() const;

signals:
    void broadcast(const QByteArray& iMessage);

public slots:
    void initSocket(const qintptr socketDescriptor);
    void send(const QByteArray& iPacket);

private slots:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState socketState);
    void readyRead();

    void sendHeartBeat();

private:
    void writePacket(const ePacketType iPacketType, const QByteArray& iPayload);

private:
    // Heartbeat
    const uint32_t mHEARTBEAT_INTERVAL_SECOND;
    QTimer* mHeartBeatTimer;
    uint16_t mHeartBeatCount;

    // Message parsing
    QByteArray mBuffer;
};

#endif // CLIENTSOCKET_H
