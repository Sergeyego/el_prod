#ifndef HTTPSYNCMANAGER_H
#define HTTPSYNCMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <QEventLoop>
#include <QApplication>
#include "rest/restconnection.h"
#include "rest/restlogin.h"

class HttpSyncManager : public QObject
{
    Q_OBJECT
public:
    explicit HttpSyncManager(QObject *parent = nullptr);
    static bool sendRequest(QString path, QString req, const QByteArray &body, QByteArray &respData, QString content_type = QString());
    static bool sendGet(QString path, QByteArray &data);

};

#endif // HTTPSYNCMANAGER_H
