#include "restlogin.h"
#include "ui_restlogin.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>

RestLogin::RestLogin(const QString &title, QWidget *parent)
    : QDialog(parent), ui(new Ui::RestLogin)
{
    ui->setupUi(this);
    setWindowTitle(title);

    // Восстанавливаем последний хост, иначе дефолт
    QSettings s("szsm", QApplication::applicationName());
    const QString lastHost = s.value("lastHost", "https://127.0.0.1").toString();
    QUrl u(lastHost);
    if (u.isValid() && !u.host().isEmpty()) {
        ui->edtHost->setText(u.scheme() + "://" + u.host());
        if (u.port() > 0) ui->edtPort->setValue(u.port());
    } else {
        ui->edtHost->setText("https://127.0.0.1");
        ui->edtPort->setValue(9000);
    }

    ui->groupBox->setVisible(false);

    connect(ui->cmdShowOpt, &QToolButton::toggled,
            ui->groupBox, &QWidget::setVisible);
    connect(ui->cmdConnect, &QPushButton::clicked,
            this, &RestLogin::restconnect);
}

RestLogin::~RestLogin()
{
    delete ui;
}

QString RestLogin::currentUrl() const
{
    QUrl u(ui->edtHost->text());
    if (!u.isValid() || u.scheme().isEmpty()) {
        u = QUrl("https://" + ui->edtHost->text());
    }
    if (u.port() == -1) {
        u.setPort(ui->edtPort->value());
    }
    u.setPath(QString());
    u.setQuery(QString());
    return u.toString(QUrl::StripTrailingSlash);
}

void RestLogin::restconnect()
{
    if (ui->edtUser->text().isEmpty() || ui->edtPasswd->text().isEmpty()) {
        QMessageBox::warning(this, tr("Вход"),
                             tr("Введите логин и пароль."),
                             QMessageBox::Ok);
        return;
    }

    ui->cmdConnect->setEnabled(false);
    ui->cmdShowOpt->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    const QJsonObject object{
        {"username", ui->edtUser->text()},
        {"password", ui->edtPasswd->text()}
    };
    const QByteArray body = QJsonDocument(object).toJson();

    QNetworkReply *reply = RestConnection::instance()->sendRequest(
        QUrl(currentUrl() + "/login"), "POST", body);

    connect(reply, &QNetworkReply::finished, this, &RestLogin::onResult);
}

void RestLogin::onResult()
{
    QApplication::restoreOverrideCursor();
    ui->cmdConnect->setEnabled(true);
    ui->cmdShowOpt->setEnabled(true);

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;

    const QByteArray data = reply->readAll();
    const QString errStr  = reply->errorString();
    const auto err        = reply->error();
    reply->deleteLater();

    if (err != QNetworkReply::NoError) {
        QMessageBox::critical(QApplication::activeWindow(),
                              tr("Ошибка"), errStr + "\n" + data,
                              QMessageBox::Cancel);
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        QMessageBox::critical(QApplication::activeWindow(),
                              tr("Ошибка"), tr("Некорректный ответ сервера."),
                              QMessageBox::Cancel);
        return;
    }

    const QJsonObject obj = doc.object();
    const QString token = obj.value("token").toString();
    const QString user  = obj.value("username").toString();
    const qint64 iat    = obj.value("iat").toVariant().toLongLong();
    const qint64 exp    = obj.value("exp").toVariant().toLongLong();

    if (token.isEmpty()) {
        QMessageBox::critical(QApplication::activeWindow(),
                              tr("Ошибка"), tr("Токен не получен."),
                              QMessageBox::Cancel);
        return;
    }

    RestConnection::instance()->setUrl(currentUrl());
    RestConnection::instance()->setToken(token, user, iat, exp);

    QSettings s("szsm", QApplication::applicationName());
    s.setValue("lastHost", currentUrl());

    accept();
}

void RestLogin::setUser(const QString &user)       { ui->edtUser->setText(user); }
void RestLogin::setPassword(const QString &pass)   { ui->edtPasswd->setText(pass); }
void RestLogin::setHost(const QString &host)       { ui->edtHost->setText(host); }
void RestLogin::setPort(int port)                  { ui->edtPort->setValue(port); }
