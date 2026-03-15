#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QThreadPool>
#include <QThread>

#include "clientsocket.h"
#include "clientthread.h"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    virtual ~Server();

    const QList<QHostAddress> getIPv4Addresses() const;
signals:

public slots:
    void start(const QString& iAddress, const quint16 iPort);
    void quit();

private slots:
    void disconnectClient();
    void broadcast(const QByteArray& iMessage);

private:
    std::list<ClientThread*> mClientThreads;

protected:
    virtual void incomingConnection(qintptr socketDescriptor) override;

};

#endif // SERVER_H
