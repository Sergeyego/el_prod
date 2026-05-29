#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tabManager = new TabManager(ui->tabWidget,this);

    actAction(ui->actionPart,&MainWindow::newFormPart,1);
    actAction(ui->actionReport,&MainWindow::newFormReport,4);
    actAction(ui->actionPack,&MainWindow::newFormPack,4);
    actAction(ui->actionPerePack,&MainWindow::newFormPerePack,4);
    actAction(ui->actionSelf,&MainWindow::newFormSelf,4);
    actAction(ui->actionFix,&MainWindow::newFormFix,4);

    loadAnalytics();

    tabManager->loadSettings();
    loadSettings();
}

MainWindow::~MainWindow()
{
    tabManager->saveSettings();
    saveSettings();
    delete ui;
}

void MainWindow::actAction(QAction *a, void (MainWindow::*sl)(), int lev)
{
    if (RestConnection::instance()->groups().contains(lev)){
        connect(a, &QAction::triggered, this, sl);
        tabManager->actAction(a);
    } else {
        a->setEnabled(false);
        a->deleteLater();
    }
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
    QString dirPath=(QDir::homePath()+"/.szsm/cash");
    QDir dir(dirPath);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    dir.mkpath(dirPath);
}

void MainWindow::loadAnalytics()
{
    QByteArray data;
    bool ok = RestConnection::instance()->sendSyncGet("api/olap/prog/"+QApplication::applicationName(),data);
    if (ok){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray arr=doc.array();
        for (const QJsonValue &value : arr) {
            int id=value.toObject().value("id").toInt();
            QString nam=value.toObject().value("nam").toString();
            QAction *act = ui->menu_analytics->addAction(nam);
            act->setProperty("id_olap",id);
            actAction(act,&MainWindow::newAnalytics,4);
        }
    }
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

void MainWindow::newAnalytics()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action){
        int id = action->property("id_olap").toInt();
        if (!tabManager->exist(sender()) && id>0){
            tabManager->addSubWindow(new CubeWidget(id),sender());
        }
    }
}

void MainWindow::newFormPack()
{
    if (!tabManager->exist(sender())){
        tabManager->addSubWindow(new FormPack(),sender());
    }
}

void MainWindow::newFormPerePack()
{
    if (!tabManager->exist(sender())){
        tabManager->addSubWindow(new FormPerePack(),sender());
    }
}

void MainWindow::newFormSelf()
{
    if (!tabManager->exist(sender())){
        tabManager->addSubWindow(new FormSelf(),sender());
    }
}

void MainWindow::newFormFix()
{
    if (!tabManager->exist(sender())){
        tabManager->addSubWindow(new FormFix(),sender());
    }
}
