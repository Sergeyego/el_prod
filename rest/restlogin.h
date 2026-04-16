#ifndef RESTLOGIN_H
#define RESTLOGIN_H

#include <QDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
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

private:
    Ui::RestLogin *ui;
    QNetworkAccessManager *manager;
    QString currentUrl();

private slots:
    void restconnect();
    void onResult(QNetworkReply *reply);
};

#endif // RESTLOGIN_H
