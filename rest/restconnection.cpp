#include "restconnection.h"

RestConnection *RestConnection::instance()
{
    static RestConnection _instance;
    return &_instance;
}

RestConnection::RestConnection(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);

    connect(manager, &QNetworkAccessManager::authenticationRequired,
            this, [](QNetworkReply *, QAuthenticator *) {
                // Оставляем пустым, чтобы проигнорировать стандартный запрос логина/пароля от Qt
            });
}

RestConnection::~RestConnection() = default;

void RestConnection::setUrl(const QString &u) { _url = u; }
QString RestConnection::getUrl() const { return _url; }
QString RestConnection::getUser() const { return currentUser; }
const QSet<int> &RestConnection::groups() const { return _groups; }

void RestConnection::setCaCertificate(const QString &path)
{
    const auto certs = QSslCertificate::fromPath(path, QSsl::Pem);
    if (certs.isEmpty()) {
        qWarning() << "[RestConnection] Не удалось загрузить CA-сертификат:" << path;
        return;
    }
    _caCert = certs.first();
}

QUrl RestConnection::buildUrl(const QString &path, const QUrlQuery &query) const
{
    QString cleanPath = path;
    QUrlQuery embeddedQuery;

    const int qPos = path.indexOf(QLatin1Char('?'));
    if (qPos >= 0) {
        cleanPath = path.left(qPos);
        embeddedQuery.setQuery(path.mid(qPos + 1));
    }

    QUrl url(_url);
    QString base = url.path();
    while (base.endsWith(QLatin1Char('/'))) base.chop(1);

    QString suffix = cleanPath;
    while (suffix.startsWith(QLatin1Char('/'))) suffix.remove(0, 1);

    url.setPath(base + QLatin1Char('/') + suffix);

    QUrlQuery finalQuery = embeddedQuery;
    if (!query.isEmpty()) {
        const auto items = query.queryItems();
        for (const auto &pair : items) {
            finalQuery.addQueryItem(pair.first, pair.second);
        }
    }
    if (!finalQuery.isEmpty()) {
        url.setQuery(finalQuery);
    }

    return url;
}

RestConnection::Method RestConnection::parseMethod(const QString &req)
{
    const QString m = req.trimmed().toUpper();
    if (m == "GET")    return Method::Get;
    if (m == "POST")   return Method::Post;
    if (m == "PUT")    return Method::Put;
    if (m == "DELETE") return Method::Delete;
    if (m == "PATCH")  return Method::Patch;
    return Method::Invalid;
}

// Установка токена и мгновенная синхронная загрузка групп
void RestConnection::setToken(const QString &t, const QString &user, qint64 from, qint64 to)
{
    token = t;
    currentUser = user;
    iat = from;
    exp = to;
    local_iat = QDateTime::currentSecsSinceEpoch();

    loadGroupsSync();
}

// Автономная проверка токена. Сама решает, когда показать окно
QString RestConnection::getToken()
{
    static bool isLocking = false;
    if (isLocking) return token;
    if (!iat || !exp) return token;   // до логина — пустой токен

    const qint64 now = QDateTime::currentSecsSinceEpoch();
    const qint64 dt  = static_cast<qint64>((exp - iat) * 0.92);

    if (now <= (local_iat + dt)) return token;

    // 1. Возвращаем RAII для флага блокировки (надежно и лаконично)
    struct Guard {
        bool &f;
        Guard(bool &flag) : f(flag) { f = true; }
        ~Guard() { f = false; }
    } guard(isLocking);

    token.clear();

    RestLogin login(tr("Пожалуйста, авторизуйтесь"));
    const QUrl url(_url);
    login.setHost(url.scheme() + "://" + url.host());
    login.setPort(url.port());

    if (login.exec() != QDialog::Accepted) {
        emit sessionExpired();

        // 2. Страховка через singleShot: даем Qt выйти из вложенных циклов exec()
        // и только потом завершаем приложение, избегая крашей при очистке стека.
        QTimer::singleShot(0, []() {
            QApplication::exit(1);
        });

        return QString();
    }

    return token;
}

QNetworkReply *RestConnection::dispatch(const QUrl &url, Method m,
                                        const QByteArray &body,
                                        QHttpMultiPart *multipart,
                                        const QString &contentType)
{
    // 1. Если метод невалидный — сразу возвращаем CancelledReply (вместо nullptr)
    if (m == Method::Invalid) {
        qWarning() << "[RestConnection] Invalid HTTP method — запрос отменен:" << url;
        return new CancelledReply(this);
    }

    // 2. Multipart поддерживается только для POST/PUT
    if (multipart && m != Method::Post && m != Method::Put) {
        qWarning() << "[RestConnection] multipart поддерживается только для POST/PUT — запрос отменен";
        return new CancelledReply(this);
    }

    QNetworkRequest request(url);
    request.setRawHeader("Accept-Charset", "UTF-8");
    request.setRawHeader("User-Agent", "Appszsm");

    const QString currentToken = getToken();
    if (!currentToken.isEmpty()) {
        request.setRawHeader("Authorization", "Bearer " + currentToken.toUtf8());
    }

    if (!multipart && m != Method::Get && m != Method::Delete) {
        QString effectiveContentType = contentType;
        if (effectiveContentType.isEmpty() && !body.isEmpty()) {
            effectiveContentType = QStringLiteral("application/json");
        }
        if (!effectiveContentType.isEmpty()) {
            request.setHeader(QNetworkRequest::ContentTypeHeader, effectiveContentType);
        }
    }

    QNetworkReply *reply = nullptr;

    if (multipart) {
        if (m == Method::Post) {
            reply = manager->post(request, multipart);
        } else if (m == Method::Put) {
            reply = manager->put(request, multipart);
        }
    } else {
        switch (m) {
        case Method::Get:    reply = manager->get(request); break;
        case Method::Post:   reply = manager->post(request, body); break;
        case Method::Put:    reply = manager->put(request, body); break;
        case Method::Delete: reply = manager->deleteResource(request); break;
        case Method::Patch:  reply = manager->sendCustomRequest(request, "PATCH", body); break;
        default:             break;
        }
    }

    // 3. Жесткая страховка: если по какой-то причине reply остался равен nullptr,
    // подменяем его на безопасный CancelledReply, чтобы приложение никогда не упало.
    if (!reply) {
        qWarning() << "[RestConnection] Не удалось создать QNetworkReply для" << url;
        return new CancelledReply(this);
    }

    if (_caCert.isNull()) {
        reply->ignoreSslErrors();
    }

    return reply;
}

QNetworkReply *RestConnection::sendRequest(const QUrl &url, const QString &req, const QByteArray &body, const QString &contentType)
{
    return dispatch(url, parseMethod(req), body, nullptr, contentType);
}

QNetworkReply *RestConnection::sendRequest(const QUrl &url, const QString &req, QHttpMultiPart *multipart)
{
    return dispatch(url, parseMethod(req), {}, multipart, {});
}

QNetworkReply *RestConnection::sendGet(const QUrl &url)
{
    return sendRequest(url, "GET");
}

bool RestConnection::sendSyncRequest(const QString &path, const QString &req,
                                     const QByteArray &body, QByteArray &respData,
                                     const QString &contentType, int timeoutMs)
{
    const Method m = parseMethod(req);
    if (m == Method::Invalid) {
        respData.clear();
        return false;
    }

    QNetworkReply *reply = dispatch(buildUrl(path), m, body, nullptr, contentType);
    if (!reply) {
        respData.clear();
        return false;
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(timeoutMs);

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, [reply]() {
        if (!reply->isFinished()) reply->abort();
    });

    timer.start();
    if (!reply->isFinished()) {
        loop.exec(QEventLoop::ExcludeUserInputEvents);
    }

    respData = reply->readAll();
    const auto err = reply->error();
    const QString errStr = reply->errorString();
    reply->deleteLater();

    if (err != QNetworkReply::NoError) {
        if (err != QNetworkReply::OperationCanceledError) {
            QMessageBox::critical(QApplication::activeWindow(), tr("Ошибка"), errStr + "\n" + respData, QMessageBox::Cancel);
        }
        return false;
    }
    return true;
}

bool RestConnection::sendSyncGet(const QString &path, QByteArray &data, int timeoutMs)
{
    return sendSyncRequest(path, "GET", {}, data, {}, timeoutMs);
}

// Единый метод парсинга JSON для синхронного и асинхронного вариантов
void RestConnection::parseGroupsJson(const QByteArray &resp)
{
    const QJsonDocument doc = QJsonDocument::fromJson(resp);
    if (doc.isNull()) return;

    _groups.clear();
    for (const QJsonValue &val : doc.array()) {
        _groups.insert(val.toObject().value("id_group").toInt());
    }
    emit groupsChanged();
}

bool RestConnection::loadGroupsSync()
{
    QNetworkReply *reply = sendRequest(buildUrl("api/groups"), "GET");
    if (!reply) return false;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(5000); // 5 секунд таймаута для критичного старта

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, [reply]() {
        if (!reply->isFinished()) reply->abort();
    });

    timer.start();
    if (!reply->isFinished()) {
        loop.exec(QEventLoop::ExcludeUserInputEvents);
    }

    const QByteArray resp = reply->readAll();
    const auto err = reply->error();
    reply->deleteLater();

    if (err != QNetworkReply::NoError) return false;

    parseGroupsJson(resp);
    return true;
}

void RestConnection::refreshGroups()
{
    QNetworkReply *reply = sendRequest(buildUrl("api/groups"), "GET");
    if (!reply) return;

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            parseGroupsJson(reply->readAll());
        }
    });
}
