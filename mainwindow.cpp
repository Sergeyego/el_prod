#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    model = new RestTableModel("test",this);

    //model->setPath("api/elrtr/parti");

    model->setFilter(model->tableName()+".id>0");

    model->setDefaultValue(1,"1111");
    model->setHeaderData(0,Qt::Horizontal,tr("id_part"),Qt::DisplayRole);

    relModel = RelModels::instance()->getModel("mark");
    ui->comboBox->setModel(relModel);
    ui->comboBox->setModelColumn(1);

    ui->tableView->setModel(model);

    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(getData()));

    connect(ui->pushButtonRel,SIGNAL(clicked(bool)),relModel,SLOT(refresh()));

    connect(model,SIGNAL(sigAboutToBeInsert()),this,SLOT(setDefaultValue()));
}

MainWindow::~MainWindow()
{
    //qDebug()<<"delete!";
    delete ui;
}

void MainWindow::getData()
{
    model->select();
    ui->tableView->resizeToContents();
}

void MainWindow::setDefaultValue()
{
    model->setDefaultValue(1,"2222");
    model->setDefaultValue(6,QDate::currentDate());
}
