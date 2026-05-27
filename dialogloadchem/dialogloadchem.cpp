#include "dialogloadchem.h"
#include "ui_dialogloadchem.h"

DialogLoadChem::DialogLoadChem(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLoadChem)
{
    ui->setupUi(this);

    fileModel = new QFileSystemModel(this);
    fileModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    fileModel->setRootPath(QDir::rootPath());
    QStringList l;
    l<<"*.xml"<<"*.XML";
    fileModel->setNameFilters(l);
    fileModel->sort(3,Qt::DescendingOrder);
    ui->tableView->setModel(fileModel);
    ui->tableView->setColumnHidden(1,true);
    ui->tableView->setColumnHidden(2,true);
    ui->tableView->setColumnWidth(0,250);
    ui->tableView->setColumnWidth(3,130);

    QSettings settings("szsm", QApplication::applicationName());
    QString dir=settings.value("resultspath",QDir::homePath()).toString();
    if (dir.isEmpty()){
        dir=QDir::homePath();
    }
    ui->lineEditDir->setText(dir);
    QTimer::singleShot(100,this,SLOT(updDir()));

    connect(ui->toolButtonDir,SIGNAL(clicked(bool)),this,SLOT(setCurrentDir()));
    connect(ui->tableView->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(parceXml(QModelIndex)));
}

DialogLoadChem::~DialogLoadChem()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("resultspath",ui->lineEditDir->text());

    delete ui;
}

double DialogLoadChem::chemVal(QString chem)
{
    return chemMap.value(chem,0.0);
}

QMap<QString, double> DialogLoadChem::chemVals()
{
    return chemMap;
}

void DialogLoadChem::loadXmlIds(QXmlStreamReader &reader)
{
    QMap <QString,QString> map;
    while (!reader.atEnd() && !(reader.name() == QLatin1String("SampleIDs") && reader.tokenType() == QXmlStreamReader::EndElement)) {
        if (reader.readNextStartElement()) {
            if (reader.name() == QLatin1String("SampleID")) {
                QString param, value;
                while (!reader.atEnd() && !(reader.name() == QLatin1String("SampleID") && reader.tokenType() == QXmlStreamReader::EndElement)) {
                    if (reader.readNextStartElement()) {
                        if (reader.name()==QLatin1String("IDName")){
                            param=loadXmlText(reader,QLatin1String("IDName"));
                        } else if (reader.name()==QLatin1String("IDValue")){
                            value=loadXmlText(reader,QLatin1String("IDValue"));
                        }
                    }
                }
                map.insert(param,value);
            }
        }
    }
    ui->lineEditMark->setText(map.value("Marka"));
    ui->lineEditDiam->setText(map.value("Diam"));
    ui->lineEditParti->setText(map.value("Parti"));
}

void DialogLoadChem::loadXmlStat(QXmlStreamReader &reader)
{
    chemMap.clear();
    while (!reader.atEnd() && !(reader.name() == QLatin1String("MeasurementStatistics") && reader.tokenType() == QXmlStreamReader::EndElement)) {
        if (reader.readNextStartElement()) {
            if (reader.name() == QLatin1String("Elements")) {
                while (!reader.atEnd() && !(reader.name() == QLatin1String("Elements") && reader.tokenType() == QXmlStreamReader::EndElement)) {
                    if (reader.readNextStartElement()) {
                        if (reader.name()==QLatin1String("Element")){
                            QString chem, value;
                            QXmlStreamAttributes a= reader.attributes();
                            chem=a.value("ElementName").toString();
                            while (!reader.atEnd() && !(reader.name() == QLatin1String("Element") && reader.tokenType() == QXmlStreamReader::EndElement)) {
                                if (reader.readNextStartElement()) {
                                    if (reader.name()==QLatin1String("ElementResult")){
                                        QXmlStreamAttributes a= reader.attributes();
                                        QString statType=a.value("StatType").toString();
                                        if (statType==QLatin1String("Reported")){
                                            while (!reader.atEnd() && !(reader.name() == QLatin1String("ElementResult") && reader.tokenType() == QXmlStreamReader::EndElement)) {
                                                if (reader.readNextStartElement()) {
                                                    if (reader.name()==QLatin1String("ResultValue")){
                                                        value=loadXmlText(reader,QLatin1String("ResultValue"));
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                            }
                            bool ok;
                            double dv=value.toDouble(&ok);
                            double rw=0.0;
                            if (ok){
                                rw=round(dv*1000)/1000.0;
                            }
                            chemMap.insert(chem,rw);
                        }

                    }
                }
            }
        }
    }
}

QString DialogLoadChem::loadXmlText(QXmlStreamReader &reader, QString tag)
{
    QString t;
    while (!reader.atEnd() && !(reader.name() == tag && reader.tokenType() == QXmlStreamReader::EndElement)) {
        reader.readNext();
        t+=reader.text().toString();
    }
    return t;
}

void DialogLoadChem::setCurrentDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, QString::fromUtf8("Выберите папку"),ui->lineEditDir->text(),QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()){
        setCurrentDir(dir);
    }
}

void DialogLoadChem::setCurrentDir(QString path)
{
    QModelIndex ind=fileModel->setRootPath(path);
    ui->tableView->setRootIndex(ind);
    ui->lineEditDir->setText(path);
}

void DialogLoadChem::updDir()
{
    setCurrentDir(ui->lineEditDir->text());
}

void DialogLoadChem::parceXml(QModelIndex index)
{
    QString path = fileModel->fileInfo(index).absoluteFilePath();
    ui->lineEditDiam->clear();
    ui->lineEditMark->clear();
    ui->lineEditParti->clear();
    ui->tableWidgetResult->clear();
    chemMap.clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return;
    }
    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()){
        xml.readNextStartElement();
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name()==QLatin1String("SampleIDs")){
                loadXmlIds(xml);
            } else if (xml.name()==QLatin1String("MeasurementStatistics")){
                loadXmlStat(xml);
            }
        }
    }
    file.close();

    ui->tableWidgetResult->setRowCount(chemMap.count());
    ui->tableWidgetResult->setColumnCount(2);
    ui->tableWidgetResult->setHorizontalHeaderItem(0,new QTableWidgetItem(QString::fromUtf8("Элем.")));
    ui->tableWidgetResult->setHorizontalHeaderItem(1,new QTableWidgetItem(QString::fromUtf8("Сод., %")));
    QMap<QString, double>::const_iterator i = chemMap.constBegin();
    int n=0;
    while (i != chemMap.constEnd()) {
        QTableWidgetItem *valitem = new QTableWidgetItem(QLocale().toString(i.value(),'f',3));
        valitem->setTextAlignment(Qt::AlignRight);
        ui->tableWidgetResult->setItem(n,0,new QTableWidgetItem(i.key()));
        ui->tableWidgetResult->setItem(n,1,valitem);
        ++i;
        ++n;
    }
}
