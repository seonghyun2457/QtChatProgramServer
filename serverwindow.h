#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QWidget>

#include "server.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ServerWindow;
}
QT_END_NAMESPACE

class ServerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ServerWindow(QWidget *parent = nullptr);
    virtual ~ServerWindow();

signals:

private:
    // Server
    Server mServer;

    // Widgets
    std::unique_ptr<Ui::ServerWindow> mUi;
};

#endif // SERVERWINDOW_H
