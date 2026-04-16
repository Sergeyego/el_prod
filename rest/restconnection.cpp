#include "restconnection.h"

RestConnection* RestConnection::connection_instance=nullptr;

RestConnection *RestConnection::instance()
{
    if (connection_instance==nullptr)
        connection_instance = new RestConnection();
    return connection_instance;
}

void RestConnection::setUrl(const QString &u)
{
    url=u;
}

void RestConnection::setToken(const QString &t)
{
    token=t;
}

QString RestConnection::getUrl() const
{
    return url;
}

QString RestConnection::getToken() const
{
    return token;
}


RestConnection::RestConnection(QObject *parent) : QObject(parent)
{

}

