#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tabManager = new TabManager(ui->tabWidget,this);

    actAction(ui->actionPart,&MainWindow::newFormPart);

    /*model = new RestTableModel("test",this);

    //model->setPath("api/elrtr/parti");

    model->setFilter(model->tableName()+".id>0");

    //model->setDefaultValue("val_int","1111");
    model->setHeaderData(0,Qt::Horizontal,tr("id_part"),Qt::EditRole);

    QStringList c;
    c<<"col_date"<<"col_rel"<<"col_int"<<"id";
    model->setColumns(c);

    relModel = RelModels::instance()->getModel("mark");
    ui->comboBox->setModel(relModel);
    ui->comboBox->setModelColumn(1);

    ui->tableView->setModel(model);

    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(getData()));

    connect(ui->pushButtonRel,SIGNAL(clicked(bool)),relModel,SLOT(refresh()));

    connect(model,SIGNAL(sigAboutToBeInsert()),this,SLOT(setDefaultValue()));*/
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
