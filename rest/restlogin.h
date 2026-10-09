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
    explicit RestLogin(const QString &title, QWidget *parent = nullptr);
    ~RestLogin();
    void setUser(const QString &user);
    void setPassword(const QString &pass);
    void setHost(const QString &host);
    void setPort(int port);

private:
    Ui::RestLogin *ui;
    QString currentUrl() const;

private slots:
    void restconnect();
    void onResult();
};

#endif // RESTLOGIN_H
