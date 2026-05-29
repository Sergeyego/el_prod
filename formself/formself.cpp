#include "formself.h"
#include "ui_formself.h"

FormSelf::FormSelf(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormSelf)
{
    ui->setupUi(this);
    loadSettings();

    ui->dateEditBeg->setDate(QDate(QDate::currentDate().year(),1,1));
    ui->dateEditEnd->setDate(QDate(QDate::currentDate().year(),12,31));

    invManager = new InvoiceManager(this);

    ui->comboBoxType->setModel(RelModels::instance()->getModel("self_type"));
    colVal c;
    c.val=4;
    ui->comboBoxType->setCurrentData(c);
    ui->comboBoxType->setEditable(false);

    modelSelfData = new RestTableModel("el_parti_self_data",this);
    ui->tableViewData->setModel(modelSelfData);

    modelSelfNakl = new RestTableModel("el_parti_self",this);
    ui->tableViewNakl->setModel(modelSelfNakl);

    mapper = new RestMapper(ui->tableViewNakl);
    ui->horizontalLayoutMap->insertWidget(0,mapper);
    mapper->addMapping(ui->lineEditNum,"num");
    mapper->addMapping(ui->dateEditNakl,"dat");
    mapper->addMapping(ui->lineEditPol,"kto");
    mapper->addLock(ui->comboBoxType);
    mapper->addEmptyLock(ui->tableViewData);
    mapper->addEmptyLock(ui->pushButtonNakl);
    mapper->addEmptyLock(ui->pushButtonNaklPer);
    mapper->addLock(ui->pushButtonUpd);
    mapper->addLock(ui->dateEditBeg);
    mapper->addLock(ui->dateEditEnd);

    connect(modelSelfNakl,SIGNAL(sigAboutToBeInsert()),this,SLOT(setDefaultValue()));
    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),this,SLOT(upd()));
    connect(ui->comboBoxType,SIGNAL(currentIndexChanged(int)),this,SLOT(upd()));
    connect(mapper,SIGNAL(currentIndexChanged(int)),this,SLOT(updData(int)));
    connect(modelSelfData,SIGNAL(sigRefresh()),this,SLOT(calcSum()));
    connect(modelSelfData,SIGNAL(sigUpd()),this,SLOT(calcSum()));
    connect(ui->pushButtonNakl,SIGNAL(clicked(bool)),this,SLOT(nakl()));
    connect(ui->pushButtonNaklPer,SIGNAL(clicked(bool)),this,SLOT(naklPer()));

    upd();
}

FormSelf::~FormSelf()
{
    saveSettings();
    delete ui;
}

void FormSelf::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->ui->splitter->restoreState(settings.value("self_splitter_width").toByteArray());
}

void FormSelf::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("self_splitter_width",ui->splitter->saveState());
}

void FormSelf::setDefaultValue()
{
    int old_num=0;
    if (modelSelfNakl->rowCount()) {
        old_num=mapper->modelData(modelSelfNakl->rowCount()-1,"num").toInt();
    }
    QString num = QString::number(old_num+1);
    num=num.rightJustified(3,'0',true);
    modelSelfNakl->setDefaultValue("num",num);
    modelSelfNakl->setDefaultValue("dat",QDate::currentDate().addDays(-1));
    modelSelfNakl->setDefaultValue("kto",ui->comboBoxType->currentText().toUpper());
}

void FormSelf::refreshRels()
{
    QVector<RestTableModel*> arrMod;
    arrMod.push_back(modelSelfNakl);
    arrMod.push_back(modelSelfData);
    RelModels::instance()->updateRels(arrMod);
}

void FormSelf::upd()
{
    if (sender()==ui->pushButtonUpd){
        refreshRels();
    }
    QString begDate=ui->dateEditBeg->date().toString("yyyy-MM-dd");
    QString endDate=ui->dateEditEnd->date().toString("yyyy-MM-dd");
    int id_type=ui->comboBoxType->getCurrentData().val.toInt();
    QString filter=modelSelfNakl->tableName()+".dat between '"+begDate+"' and '"+endDate+"' and "+modelSelfNakl->tableName()+".id_cons = "+QString::number(id_type);
    modelSelfNakl->setDefaultValue("id_cons",id_type);
    modelSelfData->setDefaultValue("id_cons",id_type);
    modelSelfNakl->setFilter(filter);
    modelSelfNakl->select();
}

void FormSelf::updData(int ind)
{
    int id_nakl=mapper->modelData(ind,"id").toInt();
    modelSelfData->setFilter(modelSelfData->tableName()+".id_self="+QString::number(id_nakl));
    modelSelfData->setDefaultValue("id_self",id_nakl);
    modelSelfData->select();
}

void FormSelf::calcSum()
{
    double sum=0;
    RestTableModel *model = qobject_cast<RestTableModel *>(sender());
    if (model){
        for (int i=0; i<model->rowCount(); i++){
            sum+=model->getModelData(i,"kvo").toDouble();
        }
    }
    QString title = ui->comboBoxType->currentText();
    if (sum!=0){
        title += tr(" итого: ")+QLocale().toString(sum,'f',1)+tr(" кг");
    }
    ui->labelItogo->setText(title);
}

void FormSelf::nakl()
{
    int id_nakl=mapper->modelData(mapper->currentIndex(),"id").toInt();
    QString vid=tr("Электроды");
    QString type=tr("Цех");
    QString filename=ui->comboBoxType->currentText().toUpper()+"_"+mapper->modelData(mapper->currentIndex(),"num").toString();
    int year=mapper->modelData(mapper->currentIndex(),"dat").toDate().year();
    invManager->getInvoice("api/invoices/elrtr/self/"+QString::number(id_nakl),vid,type,filename,year);
}

void FormSelf::naklPer()
{
    QString id_type=ui->comboBoxType->getCurrentData().val.toString();
    QString vid=tr("Электроды");
    QString type=tr("Цех");
    QString filename=ui->comboBoxType->currentText().toUpper()+"_"+ui->dateEditBeg->date().toString("yyyy-MM-dd")+"_"+ui->dateEditEnd->date().toString("yyyy-MM-dd");
    int year=mapper->modelData(mapper->currentIndex(),"dat").toDate().year();
    invManager->getInvoice("api/invoices/elrtr/selfper/"+id_type+"/"+ui->dateEditBeg->date().toString("yyyy-MM-dd")+"/"+ui->dateEditEnd->date().toString("yyyy-MM-dd"),vid,type,filename,year);
}

