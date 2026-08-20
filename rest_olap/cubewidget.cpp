#include "cubewidget.h"
#include "ui_cubewidget.h"

CubeWidget::CubeWidget(QString head, QStringList axes, QString qu, int dec, QWidget *parent) :
    QWidget(parent)
{
    initial(head,axes,qu,dec);
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
    initial(nam,axes,qu,dec);
}

CubeWidget::~CubeWidget()
{
    if (currentReply) {
        currentReply->disconnect(); // отключаем сигналы, чтобы не зайти в onResult
        currentReply->abort();
        currentReply->deleteLater();
    }
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

void CubeWidget::initial(QString head, QStringList axes, QString qu, int dec)
{
    ui = new Ui::CubeWidget;
    ui->setupUi(this);
    currentReply=nullptr;
    progressDialog = new ProgressReportDialog(this);
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

    QCalendarWidget *begCalendarWidget = new QCalendarWidget(ui->dateEditBeg);
    begCalendarWidget->setFirstDayOfWeek(Qt::Monday);
    ui->dateEditBeg->setCalendarWidget(begCalendarWidget);
    ui->dateEditBeg->setDate(QDate::currentDate().addDays(-QDate::currentDate().day()+1));
    QCalendarWidget *endCalendarWidget = new QCalendarWidget(ui->dateEditEnd);
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
    QByteArray body;
    QJsonObject obj;
    obj.insert("qu",QJsonValue(squery));
    obj.insert("columns",QJsonValue(QJsonArray::fromStringList(header)));
    obj.insert("nam",QJsonValue(title));
    obj.insert("dec",QJsonValue(decimal));
    QJsonDocument bodyDoc;
    bodyDoc.setObject(obj);
    body=bodyDoc.toJson();
    if (currentReply && currentReply->isRunning()) {
        currentReply->disconnect(); // отключаем сигналы, чтобы не зайти в onResult
        currentReply->abort();
        currentReply->deleteLater();
        progressDialog->hide();
    }
    currentReply = RestConnection::instance()->sendRequest(QUrl(RestConnection::instance()->getUrl()+"/api/olap/data"),"POST",body);
    connect(currentReply, SIGNAL(finished()), this, SLOT(onResult()));
    if (sender()==ui->cmdUpd){
        progressDialog->show();
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

void CubeWidget::onResult()
{
    progressDialog->hide();
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply){
        return;
    }

    // Игнорируем штатную отмену старого запроса
    if (reply->error() == QNetworkReply::OperationCanceledError) {
        reply->deleteLater();
        // Если это был текущий указатель, зануляем его
        if (reply == currentReply) {
            currentReply = nullptr;
        }
        return;
    }

    // Сбрасываем указатель, если пришел финальный ответ
    if (reply == currentReply) {
        currentReply = nullptr;
    }

    QByteArray data=reply->readAll();

    if (reply->error()!=QNetworkReply::NoError){
        QMessageBox::critical(nullptr,tr("Ошибка"),reply->errorString()+"\n"+data,QMessageBox::Cancel);
    } else {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            QString errorString = "Ошибка JSON:" + error.errorString() + "на позиции:" + QString::number(error.offset);
            QMessageBox::critical(nullptr,tr("Ошибка"),errorString,QMessageBox::Cancel);
            quModel->clear();
        } else {
            quModel->setModelData(doc.object());
        }
        upd();
    }
    reply->deleteLater();
}
