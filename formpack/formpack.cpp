#include "formpack.h"
#include "ui_formpack.h"

FormPack::FormPack(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormPack)
{
    ui->setupUi(this);
    loadSettings();

    invManager = new InvoiceManager(this);

    ui->dateEditBeg->setDate(QDate(QDate::currentDate().year(),1,1));
    ui->dateEditEnd->setDate(QDate(QDate::currentDate().year(),12,31));

    ui->comboBoxType->setModel(RelModels::instance()->getModel("nakl_pack_type"));
    colVal cType;
    cType.val=1;
    ui->comboBoxType->setCurrentData(cType);
    ui->comboBoxType->setEditable(false);

    modelNakl = new RestTableModel("el_parti_nakl",this);
    ui->tableViewNakl->setModel(modelNakl);

    modelPack = new RestTableModel("el_parti_pack",this);
    modelBreak = new RestTableModel("el_parti_break",this);

    mapper = new RestMapper(ui->tableViewNakl);
    ui->horizontalLayoutMap->insertWidget(0,mapper);
    mapper->addMapping(ui->lineEditNum,"num");
    mapper->addMapping(ui->dateEditNakl,"dat");
    mapper->addLock(ui->comboBoxType);
    mapper->addEmptyLock(ui->tableViewNaklData);
    mapper->addEmptyLock(ui->pushButtonNakl);
    mapper->addEmptyLock(ui->pushButtonNaklPer);
    mapper->addEmptyLock(ui->pushButtonLoad);
    mapper->addLock(ui->pushButtonUpd);
    mapper->addLock(ui->dateEditBeg);
    mapper->addLock(ui->dateEditEnd);

    connect(modelNakl,SIGNAL(sigAboutToBeInsert()),this,SLOT(setDefaultValue()));
    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),this,SLOT(upd()));
    connect(ui->comboBoxType,SIGNAL(currentIndexChanged(int)),this,SLOT(upd()));
    connect(mapper,SIGNAL(currentIndexChanged(int)),this,SLOT(updData(int)));
    connect(modelPack,SIGNAL(sigUpd()),this,SLOT(calcSum()));
    connect(modelPack,SIGNAL(sigRefresh()),this,SLOT(calcSum()));
    connect(modelBreak,SIGNAL(sigUpd()),this,SLOT(calcSum()));
    connect(modelBreak,SIGNAL(sigRefresh()),this,SLOT(calcSum()));
    connect(ui->pushButtonNakl,SIGNAL(clicked(bool)),this,SLOT(nakl()));
    connect(ui->pushButtonNaklPer,SIGNAL(clicked(bool)),this,SLOT(naklPer()));
    connect(ui->pushButtonLoad,SIGNAL(clicked(bool)),this,SLOT(loadPack()));


    upd();
}

FormPack::~FormPack()
{
    saveSettings();
    delete ui;
}

void FormPack::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->ui->splitter->restoreState(settings.value("pack_splitter_width").toByteArray());
}

void FormPack::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("pack_splitter_width",ui->splitter->saveState());
}

void FormPack::setDefaultValue()
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

void FormPack::upd()
{
    /*if (sender()==ui->pushButtonUpd){
        modelPack->refreshRelsModel();
    }*/
    QString begDate=ui->dateEditBeg->date().toString("yyyy-MM-dd");
    QString endDate=ui->dateEditEnd->date().toString("yyyy-MM-dd");
    int id_type=ui->comboBoxType->getCurrentData().val.toInt();
    QString filter=modelNakl->tableName()+".dat between '"+begDate+"' and '"+endDate+"' and "+modelNakl->tableName()+".tip = "+QString::number(id_type);
    modelNakl->setDefaultValue("tip",id_type);
    modelNakl->setFilter(filter);
    modelNakl->select();
}

void FormPack::updData(int ind)
{
    int id_nakl=mapper->modelData(ind,"id").toInt();

    if (ui->comboBoxType->getCurrentData().val==1){
        modelPack->setFilter(modelPack->tableName()+".id_nakl="+QString::number(id_nakl));
        modelPack->setDefaultValue("id_nakl",id_nakl);
        modelPack->select();

        ui->pushButtonNaklPer->hide();
        ui->tableViewNaklData->setModel(modelPack);
        ui->pushButtonLoad->show();
    } else {
        modelBreak->setFilter(modelBreak->tableName()+".id_nakl="+QString::number(id_nakl));
        modelBreak->setDefaultValue("id_nakl",id_nakl);
        modelBreak->select();

        ui->pushButtonNaklPer->show();
        ui->tableViewNaklData->setModel(modelBreak);
        ui->pushButtonLoad->hide();
    }
}

void FormPack::calcSum()
{
    double sum=0;
    RestTableModel *model = qobject_cast<RestTableModel *>(sender());
    if (model){
        for (int i=0; i<model->rowCount(); i++){
            sum+=model->getModelData(i,"kvo").toDouble();
        }
    }
    QString title = ui->comboBoxType->currentText();
    if (sum>0){
        title += tr(" итого: ")+QLocale().toString(sum,'f',1)+tr(" кг");
    }
    ui->labelItogo->setText(title);
}

void FormPack::nakl()
{
    int id_nakl=mapper->modelData(mapper->currentIndex(),"id").toInt();
    QString vid=tr("Электроды");
    QString type=tr("Цех");
    QString filename=ui->comboBoxType->currentText().toUpper()+"_"+mapper->modelData(mapper->currentIndex(),"num").toString();
    int year=mapper->modelData(mapper->currentIndex(),"dat").toDate().year();
    invManager->getInvoice("api/invoices/elrtr/workshop/"+QString::number(id_nakl),vid,type,filename,year);
}

void FormPack::naklPer()
{
    QString vid=tr("Электроды");
    QString type=tr("Цех");
    QString filename=ui->comboBoxType->currentText().toUpper()+"_"+ui->dateEditBeg->date().toString("yyyy-MM-dd")+"_"+ui->dateEditEnd->date().toString("yyyy-MM-dd");
    int year=mapper->modelData(mapper->currentIndex(),"dat").toDate().year();
    invManager->getInvoice("api/invoices/elrtr/workshopper/"+ui->dateEditBeg->date().toString("yyyy-MM-dd")+"/"+ui->dateEditEnd->date().toString("yyyy-MM-dd"),vid,type,filename,year);
}

void FormPack::loadPack()
{
    DialogPackLoad d;
    d.setCurrentDate(mapper->modelData(mapper->currentIndex(),"dat").toDate());
    d.setIdNakl(mapper->modelData(mapper->currentIndex(),"id").toInt());
    if (d.exec()==QDialog::Accepted){
        modelPack->select();
    }
}