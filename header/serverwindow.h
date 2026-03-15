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

private slots:
    // ComboBox
    void on_cbIP_currentTextChanged(const QString &arg1);

    // LineEdit
    void on_lePort_textChanged(const QString &arg1);

    // Buttons
    void on_btnStart_clicked();
    void on_btnStop_clicked();
    void on_btnQuit_clicked();

private:
    // Server
    Server mServer;
    QString mIPv4;
    QString mPort;


    // Widgets
    std::unique_ptr<Ui::ServerWindow> mUi;
};

#endif // SERVERWINDOW_H
