#include "restrelmodel.h"

RestRelModel::RestRelModel(QString name, QObject *parent) : QAbstractTableModel(parent), _name(name)
{
    manager = new QNetworkAccessManager(this);
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(onResult(QNetworkReply*)));
}

QVariant RestRelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()){
        return QVariant();
    }
    if (role==Qt::EditRole || role==Qt::DisplayRole){
        return _data[index.row()][index.column()];
    }
    return QVariant();
}

int RestRelModel::rowCount(const QModelIndex &/*parent*/) const
{
    return _data.size();
}

int RestRelModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 2;
}

QString RestRelModel::getName() const
{
    return _name;
}

void RestRelModel::refresh()
{
    refreshByPattern("");
}

void RestRelModel::refreshByPattern(QString pattern)
{
    QUrlQuery query;
    query.addQueryItem("like",pattern);
    QUrl url = QUrl(RestConnection::instance()->getUrl()+"/api/autorest/relations/"+_name);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Accept-Charset", "UTF-8");
    request.setRawHeader("User-Agent", "Appszsm");
    request.setRawHeader("Authorization", "Bearer "+RestConnection::instance()->getToken().toUtf8());

    QNetworkReply *reply = manager->get(request);
    reply->ignoreSslErrors();
    reply->setProperty("pattern",pattern);
}

void RestRelModel::clear()
{
    beginResetModel();
    _data.clear();
    endResetModel();
}

void RestRelModel::onResult(QNetworkReply *reply)
{
    if (reply->error()){
        QMessageBox::critical(nullptr,tr("Ошибка"),reply->errorString()+"\n"+reply->readAll(),QMessageBox::Cancel);
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray arr=doc.array();
        beginResetModel();
        _data.clear();
        for (const QJsonValue &value : arr) {
            QVector<QVariant>row;
            row.push_back(value.toObject().value("key").toVariant());
            row.push_back(value.toObject().value("disp").toVariant());
            _data.push_back(row);
        }
        endResetModel();
        emit refreshFinished(reply->property("pattern").toString());
    }
    reply->deleteLater();
}
