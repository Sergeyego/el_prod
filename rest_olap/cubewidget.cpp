#include "cubewidget.h"
#include "ui_cubewidget.h"

CubeWidget::CubeWidget(QString head, QStringList axes, QString qu, int dec, QWidget *parent) :
    QWidget(parent)
{
    inital(head,axes,qu,dec);
}

CubeWidget::CubeWidget(int id_cube, QWidget *parent) :
    QWidget(parent)
{
    QString nam, qu;
    QStringList axes;
    int dec=3;
    QByteArray data;
    bool ok = RestConnection::instance()->sendSyncGet("api/olap/info/"+QString::number(id_cube),data);
    if (ok) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonObject obj=doc.object();
        nam=obj.value("nam").toString();
        dec=obj.value("dec").toInt();
        qu=obj.value("qu").toString();
        const QVariantList list=obj.value("columns").toArray().toVariantList();
        for (const QVariant &v : list){
            axes.push_back(v.toString());
        }
    }
    inital(nam,axes,qu,dec);
}

CubeWidget::~CubeWidget()
{
    delete ui;
}

void CubeWidget::setRange(QDate beg, QDate end, bool block)
{
    ui->dateEditBeg->setDate(beg);
    ui->dateEditEnd->setDate(end);
    ui->dateEditBeg->setReadOnly(block);
    ui->dateEditEnd->setReadOnly(block);
    updQuery();
}

void CubeWidget::setSum(double s)
{
    sum=s;
}

double CubeWidget::getSum()
{
    double s=0;
    int col=proxyModel->columnCount()-1;
    for (int i=0; i<proxyModel->rowCount(); i++){
        s+=proxyModel->data(proxyModel->index(i,col),Qt::EditRole).toDouble();
    }
    return s;
}

void CubeWidget::inital(QString head, QStringList axes, QString qu, int dec)
{
    ui = new Ui::CubeWidget;
    ui->setupUi(this);
    sum=0.0;
    query=qu;
    decimal=dec;
    ui->cmdUpd->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload)));
    ui->cmdSave->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton)));
    header=axes;
    header<<tr("Сумма");

    quModel = new RestRoTableModel(this);
    proxyModel = new ProxyDataModel(this);
    proxyModel->setSourceModel(quModel);

    QCalendarWidget *begCalendarWidget = new QCalendarWidget(this);
    begCalendarWidget->setFirstDayOfWeek(Qt::Monday);
    ui->dateEditBeg->setCalendarWidget(begCalendarWidget);
    ui->dateEditBeg->setDate(QDate::currentDate().addDays(-QDate::currentDate().day()+1));
    QCalendarWidget *endCalendarWidget = new QCalendarWidget(this);
    endCalendarWidget->setFirstDayOfWeek(Qt::Monday);
    ui->dateEditEnd->setCalendarWidget(endCalendarWidget);
    ui->dateEditEnd->setDate(QDate::currentDate());

    axisX = new AxisWidget(axes, this);
    axisY = new AxisWidget(axes, this);
    ui->groupBoxX->layout()->addWidget(axisX);
    ui->groupBoxY->layout()->addWidget(axisY);

    this->setWindowTitle(head);
    olapmodel = new OlapModel(axes,dec,this);
    ui->tableView->setModel(olapmodel);
    ui->tableView->setDefaultDecimal(decimal);
    updQuery();
    connect(ui->cmdUpd,SIGNAL(clicked()),this,SLOT(updQuery()));
    connect(ui->radioButtonSum,SIGNAL(clicked(bool)),olapmodel,SLOT(setTypeSum(bool)));
    connect(ui->radioButtonAvg,SIGNAL(clicked(bool)),olapmodel,SLOT(setTypeAvg(bool)));
    connect(ui->radioButtonMin,SIGNAL(clicked(bool)),olapmodel,SLOT(setTypeMin(bool)));
    connect(ui->radioButtonMax,SIGNAL(clicked(bool)),olapmodel,SLOT(setTypeMax(bool)));
    connect(axisX,SIGNAL(sigUpd(QStringList)),olapmodel,SLOT(setX(QStringList)));
    connect(axisY,SIGNAL(sigUpd(QStringList)),olapmodel,SLOT(setY(QStringList)));
    connect(ui->cmdSave,SIGNAL(clicked()),ui->tableView,SLOT(saveXlsx()));
    connect(olapmodel,SIGNAL(sigRefresh()),ui->tableView,SLOT(resizeToContents()));
    connect(ui->checkBoxFlt,SIGNAL(clicked(bool)),this,SLOT(fltEnable(bool)));
    connect(ui->cmdCfgFlt,SIGNAL(clicked(bool)),this,SLOT(cfgFlt()));
}

void CubeWidget::updQuery()
{
    QString squery=query;
    squery.replace(":d1","'"+ui->dateEditBeg->date().toString("yyyy-MM-dd")+"'");
    squery.replace(":d2","'"+ui->dateEditEnd->date().toString("yyyy-MM-dd")+"'");
    QString title=this->windowTitle()+tr(" с ")+ui->dateEditBeg->date().toString("dd.MM.yyyy")+tr(" по ")+ui->dateEditEnd->date().toString("dd.MM.yyyy");
    ui->tableView->setWindowTitle(title);
    QByteArray body, data;
    QJsonObject obj;
    obj.insert("qu",QJsonValue(squery));
    obj.insert("columns",QJsonValue(QJsonArray::fromStringList(header)));
    obj.insert("nam",QJsonValue(title));
    obj.insert("dec",QJsonValue(decimal));
    QJsonDocument bodyDoc;
    bodyDoc.setObject(obj);
    body=bodyDoc.toJson();
    bool ok = RestConnection::instance()->sendSyncRequest("api/olap/data","POST",body,data);
    if (ok) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        quModel->setModelData(doc.object());
        upd();
    }
}

void CubeWidget::fltEnable(bool b)
{
    ui->cmdCfgFlt->setEnabled(b);
    proxyModel->setFilterEnabled(b);
    upd();
}

void CubeWidget::upd()
{
    data_cube d;
    double sumfact=0.0;
    if (sum>0){
        sumfact=getSum();
    }
    for (int i=0; i<proxyModel->rowCount(); i++){
        l_cube l;
        for (int j=0; j<proxyModel->columnCount(); j++){
            QVariant dt=proxyModel->data(proxyModel->index(i,j),Qt::EditRole);
            if (j!=proxyModel->columnCount()-1){
               l.dims.push_back(dt.toString()+'\n');
            } else {
                double s=dt.toDouble();
                if (sum>0 && sumfact!=0.0){
                    s=s*(sum/sumfact);
                }
                l.r=s;
            }
        }
        d.push_back(l);
    }
    olapmodel->setCubeData(d);
}

void CubeWidget::cfgFlt()
{
    DialogOlapFlt d(proxyModel);
    if (d.exec()==QDialog::Accepted){
        upd();
    }
}
