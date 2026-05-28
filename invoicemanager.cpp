#include "invoicemanager.h"

InvoiceManager::InvoiceManager(QObject *parent) : QObject(parent)
{

}

void InvoiceManager::getInvoice(QString path, QString vid, QString type, QString filename, int year)
{
    QNetworkReply *reply = RestConnection::instance()->sendGet(QUrl(RestConnection::instance()->getUrl()+"/"+path));
    reply->setProperty("invoice_vid",vid);
    reply->setProperty("invoice_type",type);
    reply->setProperty("filename",filename);
    reply->setProperty("year",year);
    connect(reply,SIGNAL(finished()),this,SLOT(onResult()));
}

void InvoiceManager::onResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error()!=QNetworkReply::NoError){
        QMessageBox::critical(nullptr,tr("Ошибка"),reply->errorString()+"\n"+reply->readAll(),QMessageBox::Cancel);
    } else {
        QString vid=reply->property("invoice_vid").toString();
        QString type=reply->property("invoice_type").toString();
        QString filename=reply->property("filename").toString();
        QString year=reply->property("year").isNull() ? QString::number(QDate::currentDate().year()) : reply->property("year").toString();
        if (vid.isEmpty()){
            vid=tr("unknown");
        }
        if (type.isEmpty()){
            type=tr("unknown");
        }
        if (filename.isEmpty()){
            filename=tr("temp.docx");
        }

        QDir dir(QDir::homePath()+tr("/Накладные"));
        if (!dir.exists()) dir.mkdir(dir.path());
        dir.setPath(dir.path()+"/"+vid);
        if (!dir.exists()) dir.mkdir(dir.path());
        dir.setPath(dir.path()+"/"+type);
        if (!dir.exists()) dir.mkdir(dir.path());
        dir.setPath(dir.path()+"/"+year);
        if (!dir.exists()) dir.mkdir(dir.path());
        filename=filename.replace(QRegularExpression("[^\\w]",QRegularExpression::UseUnicodePropertiesOption), "_");
        QString totalName=dir.path()+"/"+filename+".docx";
        QFile file(totalName);
        if (file.open(QIODevice::WriteOnly)){
            file.write(reply->readAll());
            file.close();
            QFileInfo fileInfo(file);
            QDesktopServices::openUrl((QUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))));
        }
    }
    reply->deleteLater();
}
