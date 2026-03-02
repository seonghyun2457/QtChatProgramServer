#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QObject>
#include <QThread>

#include "client.h"

class ClientThread : public QThread
{
    Q_OBJECT
public:
    explicit ClientThread(qintptr socketDescriptor, QObject *parent = nullptr);
    virtual ~ClientThread() = default;

    Client* getClient() const;
signals:

private:
    std::unique_ptr<Client> mClient;
};

#endif // CLIENTTHREAD_H
