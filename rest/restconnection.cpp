#include "restconnection.h"

RestConnection* RestConnection::connection_instance=nullptr;

RestConnection *RestConnection::instance()
{
    if (connection_instance==nullptr)
        connection_instance = new RestConnection();
    return connection_instance;
}

RestConnection::~RestConnection()
{
    //qDebug()<<"delete connection!";
}

void RestConnection::setUrl(const QString &u)
{
    _url=u;
}

void RestConnection::setToken(const QString &t, const QString &user, const qint64 &from, const qint64 &to)
{
    token=t;
    currentUser=user;
    iat=from;
    exp=to;
    local_iat=QDateTime::currentSecsSinceEpoch();
}

QString RestConnection::getUrl() const
{
    return _url;
}

QString RestConnection::getToken() const
{
    if (iat && exp){
        qint64 current_time=QDateTime::currentSecsSinceEpoch();
        qint64 dt=(exp-iat)*0.92;
        if (current_time>(local_iat+dt)){
            RestLogin l(tr("Пожалуйста, авторизуйтесь"));
            if (l.exec()!=QDialog::Accepted){
                QApplication::exit();
            }
        }
    }
    return token;
}

QString RestConnection::getUser() const
{
    return currentUser;
}

QNetworkReply *RestConnection::sendRequest(QUrl &url, QString req, const QByteArray &body, QString content_type)
{
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Charset", "UTF-8");
    request.setRawHeader("User-Agent", "Appszsm");
    request.setRawHeader("Authorization", "Bearer "+this->getToken().toUtf8());
    if (!content_type.isEmpty() && req!="GET"){
        request.setRawHeader("Content-Type", content_type.toUtf8());
    }
    QNetworkReply *reply;
    if (req=="GET"){
        reply=manager->get(request);
    } else if (req=="POST"){
        reply=manager->post(request,body);
    } else if (req=="PUT"){
        reply=manager->put(request,body);
    } else if (req=="DELETE"){
        reply=manager->deleteResource(request);
    } else {
        reply=manager->sendCustomRequest(request,req.toUtf8()/*,data*/);
    }
    reply->ignoreSslErrors();
    return reply;
}

QNetworkReply *RestConnection::sendGet(QUrl &url)
{
    QByteArray body;
    return sendRequest(url,"GET",body);
}

bool RestConnection::sendSyncRequest(QString path, QString req, const QByteArray &body, QByteArray &respData, QString content_type)
{
    QUrl url(this->getUrl()+"/"+path);
    QNetworkReply *reply = this->sendRequest(url,req,body,content_type);
    QEventLoop loop;
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    if (!reply->isFinished()){
        loop.exec(QEventLoop::ExcludeUserInputEvents);
    }
    respData=reply->readAll();
    bool ok=(reply->error()==QNetworkReply::NoError);
    if (!ok){
        QMessageBox::critical(nullptr,tr("Ошибка"),reply->errorString()+"\n"+respData,QMessageBox::Cancel);
    }
    reply->deleteLater();
    return ok;
}

bool RestConnection::sendSyncGet(QString path, QByteArray &data)
{
    QByteArray body;
    return this->sendSyncRequest(path,"GET",body,data);
}


RestConnection::RestConnection(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    iat=0;
    exp=0;
    local_iat=0;
    connect(qApp,SIGNAL(aboutToQuit()),this,SLOT(deleteLater()));
}

