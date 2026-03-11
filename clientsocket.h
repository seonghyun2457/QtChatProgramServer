#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>

class Server;

class ClientSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit ClientSocket(QObject *parent = nullptr);
    virtual ~ClientSocket();

    bool isConnected() const;

signals:
    void broadcast(const QByteArray& iMessage);

public slots:
    void initSocket(const qintptr socketDescriptor);
    void send(const QByteArray& iMessage);

private slots:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState socketState);
    void readyRead();

    void sendHeartBeat();

private:
    QTimer* mHeartBeatTimer;
    uint16_t mHeartBeatCount;
};

#endif // CLIENTSOCKET_H
