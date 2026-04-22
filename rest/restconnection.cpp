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
    url=u;
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
    return url;
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


RestConnection::RestConnection(QObject *parent) : QObject(parent)
{
    iat=0;
    exp=0;
    local_iat=0;
    connect(qApp,SIGNAL(aboutToQuit()),this,SLOT(deleteLater()));
}

