#include "dialogpackload.h"
#include "ui_dialogpackload.h"

DialogPackLoad::DialogPackLoad(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPackLoad)
{
    ui->setupUi(this);
    ui->textEdit->document()->setUseDesignMetrics(true);

    modelMaster = new RestRoTableModel(this);
    ui->comboBoxMaster->setModel(modelMaster);
    currentIdNakl=-1;

    connect(ui->dateEdit,SIGNAL(dateChanged(QDate)),this,SLOT(loadDoc()));
    connect(ui->radioButtonPart,SIGNAL(clicked(bool)),this,SLOT(loadDoc()));
    connect(ui->radioButtonRab,SIGNAL(clicked(bool)),this,SLOT(loadDoc()));
    connect(ui->pushButtonPrint,SIGNAL(clicked(bool)),this,SLOT(print()));
    connect(ui->pushButtonCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(ui->pushButtonLoad,SIGNAL(clicked(bool)),this,SLOT(loadData()));
    connect(ui->comboBoxMaster,SIGNAL(currentIndexChanged(int)),this,SLOT(loadDoc()));
}

DialogPackLoad::~DialogPackLoad()
{
    delete ui;
}

void DialogPackLoad::setHtml(const QString &html)
{
    QMap <QString, QByteArray> tmpRes;
    QTextDocument tmpDoc;
    tmpDoc.setHtml(html);
    QTextBlock bl = tmpDoc.begin();
    while(bl.isValid()){
        QTextBlock::iterator it;
        for(it = bl.begin(); !(it.atEnd()); ++it){
            QTextFragment currentFragment = it.fragment();
            if(currentFragment.isValid()){
                if(currentFragment.charFormat().isImageFormat()){
                    QTextImageFormat imgFmt = currentFragment.charFormat().toImageFormat();
                    QString name=imgFmt.name();
                    if (!tmpRes.contains(name)){
                        QByteArray resp;
                        if (RestConnection::instance()->sendSyncGet(name,resp)){
                            tmpRes.insert(name,resp);
                        }
                    }
                }
            }
        }
        bl = bl.next();
    }
    ui->textEdit->document()->clear();
    for (const QString &res : tmpRes.keys()){
        ui->textEdit->document()->addResource(QTextDocument::ImageResource,res,tmpRes.value(res));
    }
    ui->textEdit->setHtml(html);
}

void DialogPackLoad::updMaster(QDate date)
{
    /*QSqlQuery query;
    query.prepare("select distinct epo.id_main_rab, ke.snam "
                  "from el_pallet_op epo "
                  "inner join kamin_empl ke on ke.id = epo.id_main_rab "
                  "where (epo.dtm)::date = :dt "
                  "order by ke.snam");
    query.bindValue(":dt",date);
    if (modelMaster->execQuery(query)){
        ui->comboBoxMaster->setModelColumn(1);
    }
    if (ui->comboBoxMaster->model()->rowCount()){
        ui->comboBoxMaster->setCurrentIndex(0);
    }*/
}

QString DialogPackLoad::currentIdMaster()
{
    QString id_master;
    if (ui->comboBoxMaster->model()->rowCount()){
        id_master=ui->comboBoxMaster->model()->data(ui->comboBoxMaster->model()->index(ui->comboBoxMaster->currentIndex(),0),Qt::EditRole).toString();
    }
    return id_master;
}

void DialogPackLoad::setCurrentDate(QDate date)
{
    ui->dateEdit->setDate(date);
}

void DialogPackLoad::setIdNakl(int id)
{
    currentIdNakl=id;
}

void DialogPackLoad::loadDoc()
{
    if (sender()==ui->dateEdit){
        updMaster(ui->dateEdit->date());
    }
    if (ui->comboBoxMaster->model()->rowCount()){
        bool by_rab=ui->radioButtonRab->isChecked();
        QString id_master=currentIdMaster();
        QString path="api/elrtr/pack/packnakl/"+ui->dateEdit->date().toString("yyyy-MM-dd")+"/1/"+id_master+"?by_rab="+(by_rab ? "true":"false");
        QByteArray data;
        bool ok=RestConnection::instance()->sendSyncGet(path,data);
        ui->pushButtonLoad->setEnabled(ok);
        if (ok){
            setHtml(data);
        } else {
            ui->textEdit->clear();
        }
    } else {
        ui->pushButtonLoad->setEnabled(false);
        ui->textEdit->clear();
    }
}

void DialogPackLoad::print()
{
    QPrinter printer;
    QPrintDialog printDialog(&printer,this);
    if (printDialog.exec()) {
        printer.setPageMargins(QMarginsF(12, 12, 12, 12));
        printer.setPageSize(QPageSize(QPageSize::A5));
        printer.setPageOrientation(QPageLayout::Portrait);
        QPainter painter(&printer);
        if (ui->textEdit->document()->size().height()<(ui->textEdit->height()*1.2)){
            QRect rect = painter.viewport();
            QSize size (rect.size());
            size.scale(ui->textEdit->document()->size().toSize(),Qt::KeepAspectRatioByExpanding);
            painter.setWindow(0,0,size.width(),size.height());
            ui->textEdit->document()->drawContents(&painter);
        } else {
            QRect paintRect=printer.pageLayout().paintRectPixels(printer.resolution());
            QRect rect = QRect(0,0,paintRect.width(),paintRect.height());
            ui->textEdit->document()->setPageSize(rect.size());
            QTextFrameFormat fmt = ui->textEdit->document()->rootFrame()->frameFormat();
            fmt.setMargin(0);
            ui->textEdit->document()->rootFrame()->setFrameFormat(fmt);
            for (int i=0; i<ui->textEdit->document()->pageCount();i++){
                painter.save();
                painter.translate(rect.left(), rect.top() - (i) * rect.height());
                QRectF clip(0, (i) * rect.height(), rect.width(), rect.height());
                ui->textEdit->document()->drawContents(&painter, clip);
                painter.restore();
                if (i!=ui->textEdit->document()->pageCount()-1){
                    printer.newPage();
                }
            }
        }
    }
}

void DialogPackLoad::loadData()
{
    /*QSqlQuery query;
    query.prepare("insert into parti_pack (id_nakl, id_part, kvo) ( "
                  "select :id_nakl, epo.id_parti, sum(epo.kvo) "
                  "from el_pallet_op epo "
                  "where (epo.dtm)::date = :dt and epo.id_main_rab = :id_master and epo.id_src = 1 "
                  "group by epo.id_parti)");
    query.bindValue(":id_nakl",currentIdNakl);
    query.bindValue(":dt",ui->dateEdit->date());
    query.bindValue(":id_master",currentIdMaster());
    if (query.exec()){
        accept();
    } else {
        QMessageBox::critical(this, tr("Ошибка"),query.lastError().text(),QMessageBox::Cancel);
    }*/
}
