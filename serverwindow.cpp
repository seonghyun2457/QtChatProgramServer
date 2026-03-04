#include "serverwindow.h"
#include "ui_serverwindow.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

ServerWindow::ServerWindow(QWidget *parent)
    : QWidget{parent}
    , mUi(std::make_unique<Ui::ServerWindow>())
{
    mUi->setupUi(this);
}

ServerWindow::~ServerWindow()
{

}
