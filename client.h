#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>

class Server;

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(qintptr socketDescriptor, QObject *parent = nullptr);
    virtual ~Client() = default;
signals:
    void transferReceivedMessage(const QByteArray& iMessage);

public slots:
    void run();
    void write(const QByteArray& iMessage);

private slots:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState socketState);
    void readyRead();

private:
    bool openTcpSocket();

private:
    std::unique_ptr<QTcpSocket> mpSocket;
    const qintptr mSocketDescriptor;
};

#endif // CLIENT_H
