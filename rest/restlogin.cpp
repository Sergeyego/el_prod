#include "restlogin.h"
#include "ui_restlogin.h"

RestLogin::RestLogin(const QString title, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RestLogin)
{
    ui->setupUi(this);
    this->setWindowTitle(title);
    ui->edtHost->setText("https://192.168.1.134");
    ui->edtPort->setValue(9000);
    ui->groupBox->setVisible(false);

    connect(ui->cmdShowOpt,SIGNAL(clicked(bool)),ui->groupBox,SLOT(setVisible(bool)));
    connect(ui->cmdConnect,SIGNAL(clicked()),this,SLOT(restconnect()));
}

RestLogin::~RestLogin()
{
    delete ui;
}

QString RestLogin::currentUrl()
{
    return ui->edtHost->text()+":"+QString::number(ui->edtPort->value());
}

void RestLogin::restconnect()
{
    QJsonDocument doc;
    QJsonObject object
        {
            {"username", ui->edtUser->text()},
            {"password", ui->edtPasswd->text()}
        };
    doc.setObject(object);
    QByteArray body=doc.toJson();
    QNetworkReply *reply = RestConnection::instance()->sendRequest(QUrl(currentUrl()+"/login"),"POST",body);
    connect(reply,SIGNAL(finished()),this,SLOT(onResult()));
}

void RestLogin::onResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply){
        return;
    }
    if (reply->error()!=QNetworkReply::NoError){
        QMessageBox::critical(nullptr,tr("Ошибка"),reply->errorString()+"\n"+reply->readAll(),QMessageBox::Cancel);
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QString token = doc.object().value("token").toString();
        QString user = doc.object().value("username").toString();
        qint64 iat = doc.object().value("iat").toVariant().toLongLong();
        qint64 exp = doc.object().value("exp").toVariant().toLongLong();
        RestConnection::instance()->setUrl(currentUrl());
        RestConnection::instance()->setToken(token,user,iat,exp);
        if (!token.isEmpty()){
            this->accept();
        } else {
            QMessageBox::critical(nullptr,tr("Ошибка"),tr("Токен не получен."),QMessageBox::Cancel);
        }
    }
    reply->deleteLater();
}
