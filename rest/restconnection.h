#ifndef RESTCONNECTION_H
#define RESTCONNECTION_H

#include <QApplication>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QUrlQuery>
#include "restlogin.h"

class CancelledReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit CancelledReply(QObject *parent = nullptr) : QNetworkReply(parent)
    {
        setError(QNetworkReply::OperationCanceledError, QStringLiteral("Session cancelled"));
        setFinished(true);

        QMetaObject::invokeMethod(this, [this]() {
            emit finished();
            deleteLater(); // ИСПРАВЛЕНО: Теперь объект гарантированно удалится сам
        }, Qt::QueuedConnection);
    }
    void abort() override {}
protected:
    qint64 readData(char *, qint64) override { return -1; }
    qint64 bytesAvailable() const override    { return 0; }
};

class RestConnection : public QObject
{
    Q_OBJECT
public:
    static RestConnection *instance();

    void setUrl(const QString &u);
    void setCaCertificate(const QString &path);

    void setToken(const QString &t, const QString &user, qint64 from, qint64 to);
    QString getToken();
    QString getUser() const;
    QString getUrl() const;
    const QSet<int> &groups() const;

    QUrl buildUrl(const QString &path, const QUrlQuery &query = {}) const;

    QNetworkReply *sendRequest(const QUrl &url, const QString &req,
                               const QByteArray &body = {},
                               const QString &contentType = {});
    QNetworkReply *sendRequest(const QUrl &url, const QString &req,
                               QHttpMultiPart *multipart);
    QNetworkReply *sendGet(const QUrl &url);

    bool sendSyncRequest(const QString &path, const QString &req,
                         const QByteArray &body, QByteArray &respData,
                         const QString &contentType = {},
                         int timeoutMs = 30000);
    bool sendSyncGet(const QString &path, QByteArray &data, int timeoutMs = 30000);

public slots:
    void refreshGroups();

signals:
    void sessionExpired();
    void groupsChanged();

private:
    enum class Method { Get, Post, Put, Delete, Patch, Invalid };

    explicit RestConnection(QObject *parent = nullptr);
    ~RestConnection() override;
    Q_DISABLE_COPY(RestConnection)

    static Method parseMethod(const QString &req);
    QNetworkReply *dispatch(const QUrl &url, Method m,
                            const QByteArray &body,
                            QHttpMultiPart *multipart,
                            const QString &contentType);

    bool loadGroupsSync();
    void parseGroupsJson(const QByteArray &resp); // Оптимизация: единый парсер JSON

    QNetworkAccessManager *manager = nullptr;
    QString _url;
    QString token;
    QString currentUser;
    qint64 iat = 0;
    qint64 exp = 0;
    qint64 local_iat = 0;
    QSet<int> _groups;
    QSslCertificate _caCert;
};

#endif // RESTCONNECTION_H
