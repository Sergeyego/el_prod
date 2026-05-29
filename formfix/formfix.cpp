#include "formfix.h"
#include "ui_formfix.h"

FormFix::FormFix(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormFix)
{
    ui->setupUi(this);

    modelDat = new RestRoTableModel(this);
    modelDat->setPath("api/elrtr/pack/fix/dats");
    ui->comboBoxDate->setModel(modelDat);

    ui->dateEditFix->setDate(QDate::currentDate().addDays(-QDate::currentDate().day()));

    modelFix = new RestTableModel("el_parti_cex_fix",this);
    modelFix->setDefaultValue("dat",ui->dateEditFix->date());
    ui->tableViewOst->setModel(modelFix);

    connect(ui->comboBoxDate,SIGNAL(currentIndexChanged(int)),this,SLOT(updFixData(int)));
    connect(ui->pushButtonFix,SIGNAL(clicked(bool)),this,SLOT(fixNewOst()));
    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),this,SLOT(updFix()));

    updFix();
}

FormFix::~FormFix()
{
    delete ui;
}

void FormFix::updFix()
{
    modelDat->selectSync();
    if (modelDat->rowCount()){
        ui->comboBoxDate->setCurrentIndex(0);
    } else {
        modelFix->select();
    }
}

void FormFix::updFixData(int index)
{
    if (index>=0){
        QDate date=ui->comboBoxDate->model()->data(ui->comboBoxDate->model()->index(index,0),Qt::EditRole).toDate();
        modelFix->setDefaultValue("dat",date);
        modelFix->setFilter(QString(modelFix->tableName()+".dat = '%1'").arg(date.toString("yyyy-MM-dd")));
        modelFix->select();
    }
}

void FormFix::fixNewOst()
{
    QDate d=ui->dateEditFix->date();
    int n=QMessageBox::question(this,tr("Подтвердите действие"),tr("Зафиксировать остатки на ")+d.toString("dd.MM.yyyy")+tr("?"),QMessageBox::Yes,QMessageBox::No);
    if (n==QMessageBox::Yes){
        QByteArray data;
        bool ok=RestConnection::instance()->sendSyncGet("api/elrtr/pack/fix/new/"+d.toString("yyyy-MM-dd"),data);
        if (ok){
            updFix();
        }
    }
}
