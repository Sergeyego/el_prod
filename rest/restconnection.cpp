#include "restconnection.h"

std::unique_ptr<RestConnection> RestConnection::connection_instance = nullptr;

RestConnection *RestConnection::instance()
{
    if (!connection_instance){
        connection_instance.reset(new RestConnection());
    }
    return connection_instance.get(); // Возвращает обычный указатель RestConnection*
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
    updGroups();
}

QString RestConnection::getUrl() const
{
    return _url;
}

QString RestConnection::getToken()
{
    static bool isLocking = false; // Защита от рекурсии
    if (isLocking) return token;

    if (iat && exp){
        qint64 current_time = QDateTime::currentSecsSinceEpoch();
        qint64 dt = (exp - iat) * 0.92;
        if (current_time > (local_iat + dt)){
            isLocking = true;
            token = "";
            RestLogin l(tr("Пожалуйста, авторизуйтесь"));
            QUrl url(_url);
            l.setHost(url.scheme() + "://" + url.host());
            l.setPort(url.port());
            if (l.exec() != QDialog::Accepted){
                QApplication::exit();
            }
            isLocking = false;
        }
    }
    return token;
}

QString RestConnection::getUser() const
{
    return currentUser;
}

QNetworkReply *RestConnection::sendRequest(QUrl url, QString req, const QByteArray &body, QString content_type)
{
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Charset", "UTF-8");
    request.setRawHeader("User-Agent", "Appszsm");

    QString currentToken = getToken();
    if (!currentToken.isEmpty()){
        request.setRawHeader("Authorization", "Bearer " + currentToken.toUtf8());
    }
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
        reply=manager->sendCustomRequest(request,req.toUtf8(),body);
    }
    reply->ignoreSslErrors();
    return reply;
}

QNetworkReply *RestConnection::sendRequest(QUrl url, QString req, QHttpMultiPart *multiPart)
{
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Charset", "UTF-8");
    request.setRawHeader("User-Agent", "Appszsm");

    QString currentToken = getToken();
    if (!currentToken.isEmpty()){
        request.setRawHeader("Authorization", "Bearer " + currentToken.toUtf8());
    }

    QNetworkReply *reply;
    if (req=="GET"){
        reply=manager->get(request);
    } else if (req=="POST"){
        reply=manager->post(request,multiPart);
    } else if (req=="PUT"){
        reply=manager->put(request,multiPart);
    } else if (req=="DELETE"){
        reply=manager->deleteResource(request);
    } else {
        reply=manager->sendCustomRequest(request,req.toUtf8(),multiPart);
    }
    reply->ignoreSslErrors();
    return reply;
}

QNetworkReply *RestConnection::sendGet(QUrl url)
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

QSet<int> RestConnection::groups() const
{
    return _groups;
}


RestConnection::RestConnection(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    iat=0;
    exp=0;
    local_iat=0;
}

void RestConnection::updGroups()
{
    _groups.clear();
    QByteArray data;
    if (sendSyncGet("api/groups",data)){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        for (const QJsonValue &val : doc.array()) {
            int id_group=val.toObject().value("id_group").toInt();
            _groups.insert(id_group);
        }
    }
}

