#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QThreadPool>
#include <QThread>

#include "client.h"
#include "clientthread.h"


class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    virtual ~Server();

    QList<QHostAddress> getIPv4Addresses() const;
signals:

public slots:
    void start(const QString& iAddress, const quint16 iPort);
    void quit();

private slots:
    void disconnectClient();

private:
    std::list<std::unique_ptr<ClientThread>> mClientThreads;

protected:
    virtual void incomingConnection(qintptr socketDescriptor) override;

private:
    void connectClient(const qintptr iSocketDescriptor);
};

#endif // SERVER_H
