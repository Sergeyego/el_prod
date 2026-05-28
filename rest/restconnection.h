#ifndef RESTCONNECTION_H
#define RESTCONNECTION_H

#include <QObject>
#include <QDateTime>
#include <QApplication>
#include <QNetworkReply>
#include "rest/restlogin.h"

class RestConnection : public QObject
{
    Q_OBJECT
public:
    static RestConnection *instance();
    ~RestConnection();
    void setUrl(const QString &u);
    void setToken(const QString &t, const QString &user, const qint64 &from, const qint64 &to);
    QString getUrl() const;
    QString getToken();
    QString getUser() const;
    QNetworkReply* sendRequest(QUrl url, QString req, const QByteArray &body, QString content_type="application/json");
    QNetworkReply* sendGet(QUrl &url);
    bool sendSyncRequest(QString path, QString req, const QByteArray &body, QByteArray &respData, QString content_type = "application/json");
    bool sendSyncGet(QString path, QByteArray &data);

protected:
    explicit RestConnection(QObject *parent = nullptr);

private:
    static RestConnection *connection_instance;
    QString _url;
    QString token;
    QString currentUser;
    qint64 iat;
    qint64 exp;
    qint64 local_iat;
    QNetworkAccessManager *manager;
};

#endif // RESTCONNECTION_H
