#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QObject>
#include <QThread>

#include "clientsocket.h"

class ClientThread : public QThread
{
    Q_OBJECT
public:
    explicit ClientThread(qintptr socketDescriptor, QObject *parent = nullptr);
    virtual ~ClientThread() = default;

    ClientSocket* getClientSocket() const;
signals:

private:
    ClientSocket* mClientSocket;
};

#endif // CLIENTTHREAD_H
