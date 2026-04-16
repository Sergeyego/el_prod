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

    manager = new QNetworkAccessManager(this);

    connect(ui->cmdShowOpt,SIGNAL(clicked(bool)),ui->groupBox,SLOT(setVisible(bool)));
    connect(ui->cmdConnect,SIGNAL(clicked()),this,SLOT(restconnect()));
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(onResult(QNetworkReply*)));
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
    QNetworkRequest request(QUrl(currentUrl()+"/login"));
    request.setRawHeader("Accept-Charset", "UTF-8");
    request.setRawHeader("User-Agent", "Appszsm");
    request.setRawHeader("Content-Type", "application/json");
    QByteArray data=doc.toJson();
    QNetworkReply *reply = manager->post(request,data);
    reply->ignoreSslErrors();
}

void RestLogin::onResult(QNetworkReply *reply)
{
    if (reply->error()){
        QMessageBox::critical(nullptr,tr("Ошибка"),reply->errorString()+"\n"+reply->readAll(),QMessageBox::Cancel);
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QString token = doc.object().value("token").toString();
        RestConnection::instance()->setUrl(currentUrl());
        RestConnection::instance()->setToken(token);
        if (!token.isEmpty()){
            this->accept();
        } else {
            QMessageBox::critical(nullptr,tr("Ошибка"),tr("Токен не получен."),QMessageBox::Cancel);
        }
    }
    reply->deleteLater();
}
