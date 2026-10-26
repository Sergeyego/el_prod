#include "restrelmodel.h"

RestRelModel::RestRelModel(QString name, QObject *parent) : QAbstractTableModel(parent), _name(name)
{
    isProcessing=false;
    _is_limited=false;
    _editable=false;
    _path="api/autorest/relations/"+_name;
    QByteArray data;
    bool ok = RestConnection::instance()->sendSyncGet("api/autorest/relinfo/"+_name,data);
    if (ok){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        _is_limited = !doc.object().value("lim").isNull();
        _editor = doc.object().value("editor").toString();
    }
}

RestRelModel::~RestRelModel()
{

}

QVariant RestRelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= _data.size() || index.column() < 0 || index.column() >= 2){
        return QVariant();
    }
    if (role==Qt::EditRole || role==Qt::DisplayRole){
        return (index.column()==0) ? _data.at(index.row()).val : _data.at(index.row()).disp;
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

QString RestRelModel::editor() const
{
    return _editor;
}

bool RestRelModel::isLimited() const
{
    return _is_limited;
}

void RestRelModel::setPath(QString p)
{
    _path=p;
}

bool RestRelModel::isEditable() const
{
    return _editable && !_editor.isEmpty();
}

void RestRelModel::setEditable(bool e)
{
    _editable=e;
}

colVal RestRelModel::getModelData(int index) const
{
    return (index>=0 && index<_data.size()) ? _data.at(index) : colVal();
}

void RestRelModel::refresh()
{
    refreshByPattern("");
}

void RestRelModel::refreshByPattern(QString pattern)
{
    QUrlQuery query;
    query.addQueryItem("like", pattern);
    QUrl url = QUrl(RestConnection::instance()->getUrl() + "/" + _path);
    url.setQuery(query);

    // ОЧИЩАЕМ очередь перед добавлением нового запроса.
    // Все старые, еще не начавшиеся поисковые запросы удаляются, так как они устарели.
    queue.clear();
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
    reply->setProperty("pattern", pattern);

    connect(reply, &QNetworkReply::finished, this, &RestRelModel::onResult);
}

void RestRelModel::onResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    const QByteArray data = reply->readAll();
    const QNetworkReply::NetworkError netError = reply->error();
    QString pattern = reply->property("pattern").toString();
    QString networkErrorString = reply->errorString();

    reply->deleteLater();

    // ОПТИМИЗАЦИЯ: Если в очереди УЖЕ появился новый запрос, текущий ответ нам больше не нужен.
    // Пропускаем парсинг и отрисовку, сразу берем актуальный запрос.
    if (!queue.isEmpty()) {
        processNextRequest();
        return;
    }

    // 1. Обработка штатной отмены запроса
    if (netError == QNetworkReply::OperationCanceledError) {
        processNextRequest();
        return;
    }

    // 2. Обработка сетевых ошибок
    if (netError != QNetworkReply::NoError) {
        clear();
        emit refreshFinished(pattern);
        RestError::instance()->push(tr("Ошибка сети"), networkErrorString + "\n" + data);
        processNextRequest();
        return;
    }

    // 3. Парсинг JSON-документа
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);

    // 4. Обработка ошибок JSON
    if (jsonError.error != QJsonParseError::NoError) {
        clear();
        emit refreshFinished(pattern);
        QString errorString = tr("Ошибка JSON: ") + jsonError.errorString() +
                              tr("\nПозиция: ") + QString::number(jsonError.offset);
        RestError::instance()->push(tr("Ошибка данных"), errorString);
        processNextRequest();
        return;
    }

    // 5. Успешное выполнение
    const QJsonArray arr = doc.array();
    QList<colVal> tmp;
    for (const QJsonValue &value : arr) {
        colVal row;
        row.val=value.toObject().value("key").toVariant();
        row.disp=value.toObject().value("disp").toString();
        tmp.push_back(row);
    }
    setMolelData(tmp);

    emit refreshFinished(pattern);

    processNextRequest();
}

int RestRelModel::findRowByKey(QVariant key) const
{
    if (key.isNull() || _data.isEmpty()) {
        return -1;
    }

    // Сравниваем через toString(), чтобы int(1), double(1.0) и QString("1")
    // считались одним и тем же ключом (как это делает сервер и RestComboBox)
    const QString target = key.toString();

    for (int row = 0; row < _data.size(); ++row) {
        if (_data.at(row).val.toString() == target) {
            return row;
        }
    }

    return -1;
}

void RestRelModel::setMolelData(const QList<colVal> &data)
{
    beginResetModel();
    _data=data;
    endResetModel();
}
