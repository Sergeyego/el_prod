#include "restrelmodel.h"

RestRelModel::RestRelModel(QString name, QObject *parent) : QAbstractTableModel(parent), _name(name)
{
    isProcessing=false;
    _is_limited=false;
    _path="api/autorest/relations/"+_name;
    QByteArray data;
    bool ok = RestConnection::instance()->sendSyncGet("api/autorest/relinfo/"+_name,data);
    if (ok){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        _is_limited = !doc.object().value("lim").isNull();
    }
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

bool RestRelModel::isLimited()
{
    return _is_limited;
}

void RestRelModel::setPath(QString p)
{
    _path=p;
}

void RestRelModel::refresh()
{
    refreshByPattern("");
}

void RestRelModel::refreshByPattern(QString pattern)
{
    QUrlQuery query;
    query.addQueryItem("like",pattern);
    QUrl url = QUrl(RestConnection::instance()->getUrl()+"/"+_path);
    url.setQuery(query);

    queue.enqueue(url);
    if (!isProcessing) {
        processNextRequest();
    }
}

void RestRelModel::clear()
{
    beginResetModel();
    _data.clear();
    endResetModel();
}

void RestRelModel::processNextRequest()
{
    if (queue.isEmpty()) {
        isProcessing = false;
        return;
    }

    isProcessing = true;
    QUrl url = queue.dequeue();
    QUrlQuery query(url);
    QString pattern = query.queryItemValue("like");

    QNetworkReply *reply = RestConnection::instance()->sendGet(url);
    reply->setProperty("pattern",pattern);
    connect(reply,SIGNAL(finished()),this,SLOT(onResult()));
}

void RestRelModel::onResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply){
        return;
    }
    if (reply->error()!=QNetworkReply::NoError){
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
    processNextRequest();
}
