#include "formpart.h"
#include "ui_formpart.h"

FormPart::FormPart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormPart)
{
    ui->setupUi(this);

    loadSettings();

    ui->dateEditBeg->setDate(QDate(QDate::currentDate().year(),1,1));
    ui->dateEditEnd->setDate(QDate(QDate::currentDate().year(),12,31));

    ui->comboBoxOnly->setModel(RelModels::instance()->getModel("mark"));
    ui->comboBoxChemDev->setModel(RelModels::instance()->getModel("chem_dev"));
    colVal cDev;
    cDev.val=1;
    ui->comboBoxChemDev->setCurrentData(cDev);

    modelGlass = new RestTableModel("acc_glyba",this);
    ui->tableViewGlass->setModel(modelGlass);

    modelZam = new RestTableModel("el_parti_mix",this);
    ui->tableViewDoz->setModel(modelZam);

    modelZamBreak = new RestTableModel("el_zam_break",this);
    ui->tableViewDozDef->setModel(modelZamBreak);

    modelRab = new RestTableModel("el_parti_prod",this);
    ui->tableViewPress->setModel(modelRab);

    modelChem = new RestTableModel("el_parti_chem",this);
    modelChem->setPath("api/elrtr/chem/parti");
    modelChem->setDefaultValue("id_dev",1);
    ui->tableViewChem->setModel(modelChem);

    modelMech = new RestTableModel("el_parti_mech",this);
    modelMech->setPath("api/elrtr/mech/parti");
    ui->tableViewMech->setModel(modelMech);

    modelPart = new RestTableModel("el_parti",this);
    modelPart->setPath("api/elrtr/parti");

    ui->tableViewPart->setModel(modelPart);

    mapper = new RestMapper(ui->tableViewPart,this);

    mapper->addMapping(ui->lineEditPart,"n_s");
    mapper->addMapping(ui->dateEditPart,"dat_part");
    mapper->addMapping(ui->comboBoxMark,"id_el");
    mapper->addMapping(ui->lineEditDiam,"diam");
    mapper->addMapping(ui->comboBoxSrc,"id_ist");
    mapper->addMapping(ui->comboBoxVar,"id_var");
    mapper->addMapping(ui->lineEditPlan,"plan");
    mapper->addMapping(ui->comboBoxRcp,"id_rcp");
    mapper->addMapping(ui->comboBoxWire,"id_prfact");
    mapper->addMapping(ui->lineEditpartWire,"parti_prov");
    mapper->addMapping(ui->comboBoxPack,"id_pack");
    mapper->addMapping(ui->comboBoxLen,"id_long");
    mapper->addMapping(ui->plainTextEditNote,"prim");
    mapper->addMapping(ui->lineEditPlot,"dens");
    mapper->addMapping(ui->lineEditVyaz,"visc");
    mapper->addMapping(ui->lineEditObm,"liq_glass");
    mapper->addMapping(ui->lineEditOst,"dry_rst");
    mapper->addMapping(ui->lineEditKfmp,"kfmp");
    mapper->addMapping(ui->lineEditDl,"le");
    mapper->addMapping(ui->lineEditPokr,"lo");
    mapper->addMapping(ui->lineEditFil,"dfil");
    mapper->addMapping(ui->lineEditDel,"del");
    mapper->addMapping(ui->lineEditMassDry,"mas_dry");
    mapper->addMapping(ui->lineEditMassGl,"mas_lqgl");

    mapper->setDefaultFocus(1);
    mapper->addLock(ui->dateEditBeg);
    mapper->addLock(ui->dateEditEnd);
    mapper->addLock(ui->pushButtonUpd);
    mapper->addLock(ui->comboBoxOnly);
    mapper->addEmptyLock(ui->tableViewGlass);
    mapper->addEmptyLock(ui->tableViewGlassPar);
    mapper->addEmptyLock(ui->tableViewGlassData);
    mapper->addEmptyLock(ui->tableViewDoz);
    mapper->addEmptyLock(ui->tableViewDozDef);
    mapper->addEmptyLock(ui->tableViewPress);
    mapper->addEmptyLock(ui->tableViewChem);
    mapper->addEmptyLock(ui->tableViewMech);
    mapper->addEmptyLock(ui->pushButtonChem);
    mapper->addEmptyLock(ui->pushButtonSamp);
    mapper->addLock(ui->checkBoxOnly);
    mapper->addEmptyLock(ui->comboBoxChemDev);

    ui->horizontalLayoutMapper->insertWidget(0,mapper);

    connect(mapper,SIGNAL(currentIndexChanged(int)),this,SLOT(refreshCont(int)));
    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),this,SLOT(updPart()));
    connect(modelPart,SIGNAL(sigAboutToBeInsert()),this,SLOT(setDefaultValue()));
    connect(ui->comboBoxOnly,SIGNAL(currentIndexChanged(int)),this,SLOT(updPart()));
    connect(ui->checkBoxOnly,SIGNAL(clicked(bool)),this,SLOT(updPart()));
    connect(ui->checkBoxOnly,SIGNAL(clicked(bool)),ui->comboBoxOnly,SLOT(setEnabled(bool)));
    connect(ui->comboBoxRcp,SIGNAL(currentIndexChanged(int)),this,SLOT(insertMark()));
    connect(ui->comboBoxMark,SIGNAL(currentIndexChanged(int)),this,SLOT(insertProvol()));
    connect(ui->lineEditDiam,SIGNAL(editingFinished()),this,SLOT(insertPack()));

    connect(ui->comboBoxChemDev,SIGNAL(currentIndexChanged(int)),this,SLOT(setCurrentChemDev()));
    connect(ui->pushButtonChem,SIGNAL(clicked(bool)),this,SLOT(loadChem()));
    connect(ui->pushButtonSamp,SIGNAL(clicked(bool)),this,SLOT(insertChemSamp()));

    updPart();

    /*
    modelConsStatData = new ModelConsStatData(this);
    modelConsStatData->refresh(-1);
    ui->tableViewGlassData->setModel(modelConsStatData);
    ui->tableViewGlassData->setColumnHidden(0,true);
    ui->tableViewGlassData->setColumnWidth(1,40);
    ui->tableViewGlassData->setColumnWidth(2,70);
    ui->tableViewGlassData->setColumnWidth(3,60);
    ui->tableViewGlassData->setColumnWidth(4,70);
    ui->tableViewGlassData->setColumnHidden(5,true);

    modelConsStatPar = new ModelConsStatPar(this);
    modelConsStatPar->refresh(-1,-1);
    ui->tableViewGlassPar->setModel(modelConsStatPar);
    ui->tableViewGlassPar->setColumnHidden(0,true);
    ui->tableViewGlassPar->setColumnWidth(1,80);
    ui->tableViewGlassPar->setColumnWidth(2,60);
    ui->tableViewGlassPar->setColumnWidth(3,70);
    ui->tableViewGlassPar->setColumnWidth(4,80);

    modelPackPal = new ModelPackPal(this);
    ui->tableViewPackPal->setModel(modelPackPal);

    modelPackEl = new ModelPackEl(this);
    ui->tableViewPack->setModel(modelPackEl);

    modelThermoPack = new ModelThermoPack(this);
    ui->tableViewThermoPack->setModel(modelThermoPack);

    modelPerePackEl = new ModelPerePackEl(this);
    ui->tableViewPerePack->setModel(modelPerePackEl);

    modelStockEl = new ModelStockEl(this);
    ui->tableViewTrans->setModel(modelStockEl);

    modelSelfEl = new ModelSelfEl(this);
    ui->tableViewSelf->setModel(modelSelfEl);

    modelShipEl = new ModelShipEl(this);
    ui->tableViewShip->setModel(modelShipEl);

    modelBreakEl = new ModelBreakEl(this);
    ui->tableViewDef->setModel(modelBreakEl);

    connect(ui->tableViewGlass->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(refreshGlassData(QModelIndex)));
    */
}

FormPart::~FormPart()
{
    saveSettings();
    delete ui;
}

void FormPart::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->ui->splitter->restoreState(settings.value("part_splitter_width").toByteArray());
    ui->tabWidget->setCurrentIndex(settings.value("part_tab_index").toInt());
}

void FormPart::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("part_splitter_width",ui->splitter->saveState());
    settings.setValue("part_tab_index",ui->tabWidget->currentIndex());
}

void FormPart::setDefaultValue()
{
    int old_num=0;
    if (modelPart->rowCount()) {
        old_num=mapper->modelData(modelPart->rowCount()-1,"n_s").toInt();
    }
    QString num = QString::number(old_num+1);
    num=num.rightJustified(4,'0',true);
    modelPart->setDefaultValue("n_s",num);
    modelPart->setDefaultValue("dat_part",QDate::currentDate());
}


void FormPart::updPart()
{
    if (sender()==ui->comboBoxOnly && !ui->checkBoxOnly->isChecked()){
        return;
    }
    int id_el=-1;
    if (ui->checkBoxOnly->isChecked()){
        if (sender()==ui->checkBoxOnly){
            colVal d;
            d.val=mapper->modelData(mapper->currentIndex(),"id_el").toInt();
            ui->comboBoxOnly->setCurrentData(d);
        }
        id_el=ui->comboBoxOnly->getCurrentData().val.toInt();
    }
    /*if (sender()==ui->pushButtonUpd){
        modelPart->refreshRelsModel();
    }*/
    QString filter=modelPart->tableName()+".dat_part between '"+ui->dateEditBeg->date().toString("yyyy-MM-dd")+"' and '"+ui->dateEditEnd->date().toString("yyyy-MM-dd")+"'";
    if (id_el>0){
        filter+=" and "+modelPart->tableName()+".id_el="+QString::number(id_el);
    }
    modelPart->setFilter(filter);
    modelPart->select();
    qDebug()<<"upd!"<<sender();
}

void FormPart::refreshCont(int ind)
{
    qDebug()<<"refresh_cont!";
    int id_part=mapper->modelData(ind,"id").isNull() ? -1 : mapper->modelData(ind,"id").toInt();
    QDate dat_part=mapper->modelData(ind,"dat_part").toDate();
    //updRow();

    modelGlass->setFilter(modelGlass->tableName()+".id_part = "+QString::number(id_part));
    modelGlass->setDefaultValue("id_part",id_part);
    modelGlass->select();
    ui->tableViewGlass->setCurrentIndex(ui->tableViewGlass->model()->index(0,2));

    modelZam->setFilter(modelZam->tableName()+".id_part = "+QString::number(id_part));
    modelZam->setDefaultValue("id_part",id_part);
    modelZam->select();

    modelZamBreak->setFilter(modelZamBreak->tableName()+".id_part = "+QString::number(id_part));
    modelZamBreak->setDefaultValue("id_part",id_part);
    modelZamBreak->setDefaultValue("dat_part",dat_part);
    modelZamBreak->select();

    modelRab->setFilter(modelRab->tableName()+".id_part = "+QString::number(id_part));
    modelRab->setDefaultValue("id_part",id_part);
    modelRab->setDefaultValue("dat",dat_part);
    modelRab->select();

    modelChem->setFilter(modelChem->tableName()+".id_part = "+QString::number(id_part));
    modelChem->setDefaultValue("id_part",id_part);
    modelChem->select();

    modelMech->setFilter(modelMech->tableName()+".id_part = "+QString::number(id_part));
    modelMech->setDefaultValue("id_part",id_part);
    modelMech->select();

    /*refreshStat(ui->groupBoxPackPal,ui->tableViewPackPal);
    refreshStat(ui->groupBoxPack,ui->tableViewPack);
    refreshStat(ui->groupBoxThermoPack,ui->tableViewThermoPack);
    refreshStat(ui->groupBoxPerepack,ui->tableViewPerePack);
    refreshStat(ui->groupBoxTrans,ui->tableViewTrans);
    refreshStat(ui->groupBoxSelf,ui->tableViewSelf);
    refreshStat(ui->groupBoxShip,ui->tableViewShip);
    refreshStat(ui->groupBoxDef,ui->tableViewDef);*/
}

void FormPart::setCurrentChemDev()
{
    int id_dev = ui->comboBoxChemDev->getCurrentData().val.toInt();
    modelChem->setDefaultValue("id_dev",id_dev);
    modelChem->select();
}

void FormPart::loadChem()
{
    /*DialogLoadChem d;
    if (d.exec()==QDialog::Accepted){
        QList <int> l = modelChem->ids();
        foreach (int key,l){
            QString chem=Rels::instance()->relChem->getDisplayValue(key);
            double val=d.chemVal(chem);
            if (val>0){
                modelChem->addChem(key,val,2);
            }
        }
    }
    modelChem->select();
    modelPart->refreshState();*/
}

void FormPart::insertChemSamp()
{
    /*QList<int> ids;
    int id_dev=ui->comboBoxChemDev->getCurrentData().val.toInt();
    QList <int> l = modelChem->ids();
    foreach (int key,l){
        int id=modelChem->addChem(key,0.0,id_dev);
        ids.push_back(id);
    }
    DialogTmp dt;
    dt.load(mapper->modelData(mapper->currentIndex(),0).toInt(),id_dev,ids);
    dt.exec();
    modelChem->select();
    modelPart->refreshState();*/
}

/*void FormPart::refreshGlassData(QModelIndex index)
{

}*/

void FormPart::insertMark()
{

}

void FormPart::insertProvol()
{

}

void FormPart::insertPack()
{

}