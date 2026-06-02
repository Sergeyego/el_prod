#ifndef RESTLOGIN_H
#define RESTLOGIN_H

#include <QDialog>
#include <QMessageBox>
#include <QUrl>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include "rest/restconnection.h"

namespace Ui {
    class RestLogin;
}

class RestLogin : public QDialog
{
    Q_OBJECT

public:
    explicit RestLogin(const QString title, QWidget *parent = nullptr);
    ~RestLogin();
    void setUser(QString user);
    void setPassword(QString pass);
    void setHost(QString host);
    void setPort(int port);

private:
    Ui::RestLogin *ui;
    QString currentUrl();

private slots:
    void restconnect();
    void onResult();
};

#endif // RESTLOGIN_H
