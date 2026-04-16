#include "httpsyncmanager.h"

HttpSyncManager::HttpSyncManager(QObject *parent)
    : QObject{parent}
{

}

bool HttpSyncManager::sendRequest(QString path, QString req, const QByteArray &data, QByteArray &respData)
{
    QNetworkRequest request(QUrl(RestConnection::instance()->getUrl()+"/"+path));
    request.setRawHeader("Accept-Charset", "UTF-8");
    request.setRawHeader("User-Agent", "Appszsm");
    request.setRawHeader("Authorization", "Bearer "+RestConnection::instance()->getToken().toUtf8());
    QEventLoop loop;
    QNetworkAccessManager man;
    connect(&man,SIGNAL(finished(QNetworkReply*)),&loop,SLOT(quit()));
    QNetworkReply *reply;
    if (req=="GET"){
        reply=man.get(request);
    } else if (req=="POST"){
        reply=man.post(request,data);
    } else if (req=="DELETE"){
        reply=man.deleteResource(request);
    } else {
        reply=man.sendCustomRequest(request,req.toUtf8()/*,data*/);
    }
    reply->ignoreSslErrors();
    if (!reply->isFinished()){
        loop.exec(QEventLoop::ExcludeUserInputEvents);
    }
    respData=reply->readAll();
    bool ok=(reply->error()==QNetworkReply::NoError);
    if (!ok){
        int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status_code==401 || status_code==403){
            RestLogin l(tr("Пожалуйста, авторизуйтесь"));
            if (l.exec()!=QDialog::Accepted) {
                QApplication::exit();
            } else {
                ok=HttpSyncManager::sendRequest(path,req, data,respData);
            }
        } else {
            QMessageBox::critical(nullptr,tr("Ошибка"),reply->errorString()+"\n"+respData,QMessageBox::Cancel);
        }
    }
    reply->deleteLater();
    return ok;
}

bool HttpSyncManager::sendGet(QString path, QByteArray &data)
{
    const QByteArray qudata;
    return sendRequest(path,"GET",qudata,data);
}
