#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tabManager = new TabManager(ui->tabWidget,this);

    actAction(ui->actionPart,&MainWindow::newFormPart);
    actAction(ui->actionReport,&MainWindow::newFormReport);

    tabManager->loadSettings();
    loadSettings();
}

MainWindow::~MainWindow()
{
    tabManager->saveSettings();
    saveSettings();
    delete ui;
}

void MainWindow::actAction(QAction *a, void (MainWindow::*sl)())
{
    connect(a, &QAction::triggered, this, sl);
    tabManager->actAction(a);
}

void MainWindow::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->restoreState(settings.value("main_state").toByteArray());
    this->restoreGeometry(settings.value("main_geometry").toByteArray());
}

void MainWindow::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("main_state", this->saveState());
    settings.setValue("main_geometry", this->saveGeometry());
}

void MainWindow::newFormPart()
{
    if (!tabManager->exist(sender())){
        tabManager->addSubWindow(new FormPart(),sender());
    }
}

void MainWindow::newFormReport()
{
    if (!tabManager->exist(sender())){
        tabManager->addSubWindow(new FormReport(),sender());
    }
}
