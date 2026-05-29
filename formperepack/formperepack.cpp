#include "formperepack.h"
#include "ui_formperepack.h"

FormPerePack::FormPerePack(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormPerePack)
{
    ui->setupUi(this);
    loadSettings();

    ui->dateEditBeg->setDate(QDate(QDate::currentDate().year(),1,1));
    ui->dateEditEnd->setDate(QDate(QDate::currentDate().year(),12,31));

    invManager = new InvoiceManager(this);

    modelPerePack = new RestTableModel("el_parti_perepack",this);
    ui->tableViewData->setModel(modelPerePack);

    modelNakl = new RestTableModel("el_parti_nakl",this);
    ui->tableViewNakl->setModel(modelNakl);

    mapper = new RestMapper(ui->tableViewNakl);
    ui->horizontalLayoutMap->insertWidget(0,mapper);
    mapper->addMapping(ui->lineEditNum,"num");
    mapper->addMapping(ui->dateEditNakl,"dat");
    mapper->addEmptyLock(ui->tableViewData);
    mapper->addEmptyLock(ui->pushButtonNakl);
    mapper->addLock(ui->pushButtonUpd);
    mapper->addLock(ui->dateEditBeg);
    mapper->addLock(ui->dateEditEnd);

    connect(modelNakl,SIGNAL(sigAboutToBeInsert()),this,SLOT(setDefaultValue()));
    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),this,SLOT(upd()));
    connect(mapper,SIGNAL(currentIndexChanged(int)),this,SLOT(updData(int)));
    connect(modelPerePack,SIGNAL(sigUpd()),this,SLOT(calcSum()));
    connect(ui->pushButtonNakl,SIGNAL(clicked(bool)),this,SLOT(nakl()));
    connect(modelPerePack,SIGNAL(sigUpd()),this,SLOT(calcSum()));
    connect(modelPerePack,SIGNAL(sigRefresh()),this,SLOT(calcSum()));

    upd();
}

FormPerePack::~FormPerePack()
{
    saveSettings();
    delete ui;
}

void FormPerePack::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->ui->splitter->restoreState(settings.value("perepack_splitter_width").toByteArray());
}

void FormPerePack::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("perepack_splitter_width",ui->splitter->saveState());
}

void FormPerePack::upd()
{
    if (sender()==ui->pushButtonUpd){
        refreshRels();
    }
    QString begDate=ui->dateEditBeg->date().toString("yyyy-MM-dd");
    QString endDate=ui->dateEditEnd->date().toString("yyyy-MM-dd");
    int id_type=7;
    QString filter=modelNakl->tableName()+".dat between '"+begDate+"' and '"+endDate+"' and "+modelNakl->tableName()+".tip = "+QString::number(id_type);
    modelNakl->setDefaultValue("tip",id_type);
    modelNakl->setFilter(filter);
    modelNakl->select();
}

void FormPerePack::updData(int ind)
{
    int id_nakl=mapper->modelData(ind,"id").toInt();
    modelPerePack->setFilter(modelPerePack->tableName()+".id_nakl="+QString::number(id_nakl));
    modelPerePack->setDefaultValue("id_nakl",id_nakl);
    modelPerePack->select();
}

void FormPerePack::calcSum()
{
    double sum=0;
    RestTableModel *model = qobject_cast<RestTableModel *>(sender());
    if (model){
        for (int i=0; i<model->rowCount(); i++){
            sum+=model->getModelData(i,"kvo").toDouble();
        }
    }
    QString title = tr("Переупаковка");
    if (sum!=0){
        title += tr(" итого: ")+QLocale().toString(sum,'f',1)+tr(" кг");
    }
    ui->labelItogo->setText(title);
}

void FormPerePack::nakl()
{
    int id_nakl=mapper->modelData(mapper->currentIndex(),"id").toInt();
    QString vid=tr("Электроды");
    QString type=tr("Переупаковка");
    QString filename=tr("ПЕРЕУПАКОВКА_")+mapper->modelData(mapper->currentIndex(),"num").toString();
    int year=mapper->modelData(mapper->currentIndex(),"dat").toDate().year();
    invManager->getInvoice("api/invoices/elrtr/perepack/"+QString::number(id_nakl),vid,type,filename,year);
}

void FormPerePack::setDefaultValue()
{
    int old_num=0;
    if (modelNakl->rowCount()) {
        old_num=mapper->modelData(modelNakl->rowCount()-1,"num").toInt();
    }
    QString num = QString::number(old_num+1);
    num=num.rightJustified(3,'0',true);
    modelNakl->setDefaultValue("num",num);
    modelNakl->setDefaultValue("dat",QDate::currentDate().addDays(-1));
}

void FormPerePack::refreshRels()
{
    QVector<RestTableModel*> arrMod;
    arrMod.push_back(modelNakl);
    arrMod.push_back(modelPerePack);
    RelModels::instance()->updateRels(arrMod);
}
