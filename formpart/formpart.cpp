#include "formpart.h"
#include "ui_formpart.h"

FormPart::FormPart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormPart)
{
    ui->setupUi(this);

    isProcessing=false;

    loadSettings();

    ui->dateEditBeg->setDate(QDate(QDate::currentDate().year(),1,1));
    ui->dateEditEnd->setDate(QDate(QDate::currentDate().year(),12,31));

    manager = new QNetworkAccessManager(this);

    ui->comboBoxOnly->setModel(RelModels::instance()->getModel("mark"));
    ui->comboBoxChemDev->setModel(RelModels::instance()->getModel("chem_dev"));
    colVal cDev;
    cDev.val=1;
    ui->comboBoxChemDev->setCurrentData(cDev);

    modelConsStatData = new RestRoTableModel(this);
    ui->tableViewGlassData->setModel(modelConsStatData);

    modelConsStatPar = new RestRoTableModel(this);
    ui->tableViewGlassPar->setModel(modelConsStatPar);

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
    connect(ui->tableViewGlass->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(refreshGlassData(QModelIndex)));
    connect(modelGlass,SIGNAL(sigRefresh()),this,SLOT(selectGlass()));

    updPart();
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

void FormPart::selectGlass()
{
    ui->tableViewGlass->setCurrentIndex(ui->tableViewGlass->model()->index(0,2));
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
    //qDebug()<<"upd!"<<sender();
}

void FormPart::refreshCont(int ind)
{
    //qDebug()<<"refresh_cont!";
    int id_part=mapper->modelData(ind,"id").isNull() ? -1 : mapper->modelData(ind,"id").toInt();
    QDate dat_part=mapper->modelData(ind,"dat_part").toDate();

    modelGlass->setFilter(modelGlass->tableName()+".id_part = "+QString::number(id_part));
    modelGlass->setDefaultValue("id_part",id_part);
    modelGlass->select();

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

    QUrl url = QUrl(RestConnection::instance()->getUrl()+"/api/elrtr/stat/"+QString::number(id_part));
    queue.enqueue(url);
    if (!isProcessing) {
        processNextRequest();
    }

}

void FormPart::processNextRequest()
{
    if (queue.isEmpty()) {
        isProcessing = false;
        return;
    }

    clearStat();

    isProcessing = true;
    QUrl url = queue.dequeue();

    QNetworkRequest request(url);
    request.setRawHeader("Accept-Charset", "UTF-8");
    request.setRawHeader("User-Agent", "Appszsm");
    request.setRawHeader("Authorization", "Bearer "+RestConnection::instance()->getToken().toUtf8());
    QNetworkReply *reply = manager->get(request);
    reply->ignoreSslErrors();
    connect(reply,SIGNAL(finished()),this,SLOT(updStat()));
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

void FormPart::refreshGlassData(QModelIndex index)
{
    QString id_part = mapper->modelData(mapper->currentIndex(),"id").toString();
    QVariant id_c=ui->tableViewGlass->model()->data(ui->tableViewGlass->model()->index(index.row(),3),Qt::EditRole);

    modelConsStatData->setPath("api/elrtr/glass/"+id_part+"/"+id_c.toString());
    modelConsStatData->select();

    modelConsStatPar->setPath("api/elrtr/glasspar/"+id_part+"/"+id_c.toString());
    modelConsStatPar->select();
}

void FormPart::clearStat()
{
    QLayout* layout = ui->scrollAreaWidgetContents->layout();
    if (layout) {
        QLayoutItem* item;
        // Цикл извлекает первый элемент, пока компоновщик не опустеет
        while ((item = layout->takeAt(0)) != nullptr) {
            // Если это виджет — удаляем его
            if (item->widget()) {
                delete item->widget();
            }
            // Удаляем сам элемент компоновщика (отступы, спейсеры)
            delete item;
        }
    }
}

void FormPart::updStat()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply){
        QByteArray data=reply->readAll();
        bool ok=(reply->error()==QNetworkReply::NoError);
        if (!ok){
            QMessageBox::critical(nullptr,tr("Ошибка"),reply->errorString()+"\n"+data,QMessageBox::Cancel);
        } else {
            const QJsonDocument doc = QJsonDocument::fromJson(data);
            for (const QJsonValue &val : doc.array()){
                QJsonArray rows = val.toObject().value("rows").toArray();
                if (!rows.size()){
                    continue;
                }
                QGroupBox *box = new QGroupBox(this);
                box->setTitle(val.toObject().value("title").toString());
                RestTableView *view = new RestTableView(box);
                box->setLayout(new QVBoxLayout(box));
                box->layout()->addWidget(view);
                RestRoTableModel *model = new RestRoTableModel(view);
                view->setModel(model);
                model->setModelData(val.toObject());
                view->verticalHeader()->setVisible(false);

                box->setFlat(true);
                box->layout()->setContentsMargins(2,8,2,2);

                ui->scrollAreaWidgetContents->layout()->addWidget(box);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
                int tw=5+box->fontMetrics().horizontalAdvance(box->title());
#else
                int tw=5+box->fontMetrics().width(box->title());
#endif
                int w=30+view->verticalHeader()->frameSize().width();
                for (int i=0; i<view->model()->columnCount(); i++){
                    if (!view->isColumnHidden(i)){
                        w+=view->columnWidth(i);
                    }
                }
                w=(tw>w)? tw : w;
                box->setMinimumSize(w,0);
            }
            ui->scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
        }
        reply->deleteLater();
    }
    processNextRequest();
}


void FormPart::insertMark()
{

}

void FormPart::insertProvol()
{

}

void FormPart::insertPack()
{

}