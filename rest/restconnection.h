#ifndef RESTCONNECTION_H
#define RESTCONNECTION_H

#include <QObject>
#include <QDateTime>
#include <QApplication>
#include "rest/restlogin.h"

class RestConnection : public QObject
{
    Q_OBJECT
public:
    static RestConnection *instance();
    void setUrl(const QString &u);
    void setToken(const QString &t, const QString &user, const qint64 &from, const qint64 &to);
    QString getUrl() const;
    QString getToken() const;
    QString getUser() const;

protected:
    explicit RestConnection(QObject *parent = nullptr);

private:
    static RestConnection *connection_instance;
    QString url;
    QString token;
    QString currentUser;
    qint64 iat;
    qint64 exp;
    qint64 local_iat;
};

#endif // RESTCONNECTION_H
