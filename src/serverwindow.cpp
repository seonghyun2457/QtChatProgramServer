#include "serverwindow.h"
#include "ui_serverwindow.h"

#include <QDebug>


#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>


ServerWindow::ServerWindow(QWidget *parent)
    : QWidget{parent}
    , mIPv4("")
    , mPort("0")
    , mUi(std::make_unique<Ui::ServerWindow>())

{
    mUi->setupUi(this);

    bool firstIPv4Selected = false;
    const QList<QHostAddress> ipv4Addresses = mServer.getIPv4Addresses();
    for (const QHostAddress& ipv4 : ipv4Addresses) {
        mUi->cbIP->addItem(ipv4.toString());
        if (!firstIPv4Selected) {
            firstIPv4Selected = true;
            mIPv4 = ipv4.toString();
        }
    }

    mUi->lePort->setText(mPort);
}

ServerWindow::~ServerWindow()
{

}

void ServerWindow::on_cbIP_currentTextChanged(const QString &arg1)
{
    qDebug() << "on_cbIP_currentTextChanged";
    mIPv4 = arg1;
    qDebug() << "mIPv4: " << mIPv4;
}

void ServerWindow::on_lePort_textChanged(const QString &arg1)
{
    qDebug() << "on_lePort_textEdited";
    mPort = arg1;
    qDebug() << "mPort: " << mPort;
}

void ServerWindow::on_btnStart_clicked()
{
    qDebug() << "on_btnStart_clicked";
    QString textMsg = "";
    if (mServer.isListening()) {
        textMsg = "Server is already listening.\n\nAddress in use: " + mServer.serverAddress().toString() + "\nPort in use: " + QString::number(mServer.serverPort());
        mUi->lbStatus->setText(textMsg);
        return;
    }

    quint16 portNum = 0;
    bool ok = false;
    portNum = mPort.toUInt(&ok);

    if (!ok) {
        textMsg = "Input integer number in Port.\n\nCurrent Port value: " + mPort + ".";
        mUi->lbStatus->setText(textMsg);
        return;
    }

    mServer.start(mIPv4, portNum);
    if (mServer.isListening()) {
        textMsg = "Server is listening now.\n\nAddress: " + mServer.serverAddress().toString() + "\nPort: " + QString::number(mServer.serverPort());
        mUi->lbStatus->setText(textMsg);

        // Do not allow to change GUI
        mUi->cbIP->setDisabled(true);
        mUi->lePort->setReadOnly(true);
    } else {
        mUi->lbStatus->setText(mServer.errorString());
    }
}

void ServerWindow::on_btnStop_clicked()
{
    qDebug() << "on_btnStop_clicked";
    QString textMsg = "";

    if (mServer.isListening()) {
        mServer.quit();
        textMsg = "Server closed.";
        mUi->lbStatus->setText(textMsg);
    }

    // Allow to change GUI
    mUi->cbIP->setDisabled(false);
    mUi->lePort->setReadOnly(false);
}

void ServerWindow::on_btnQuit_clicked()
{
    qDebug() << "on_btnQuit_clicked";
    if (mServer.isListening()) {
        mServer.quit();
    }

    close();
}
