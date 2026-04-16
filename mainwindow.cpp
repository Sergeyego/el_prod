#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(getData()));
}

MainWindow::~MainWindow()
{
    qDebug()<<"delete!";
    delete ui;
}

void MainWindow::getData()
{
    QByteArray data;
    bool ok = HttpSyncManager::sendGet("api/elrtr/parti",data);
    qDebug()<<ok;
}
