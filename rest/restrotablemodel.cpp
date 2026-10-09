#include "restrotablemodel.h"

RestRoTableModel::RestRoTableModel(QObject *parent) : QAbstractTableModel{parent}
{
    isProcessing=false;
}

RestRoTableModel::~RestRoTableModel()
{

}

QVariant RestRoTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= modelData.size() || index.column() < 0 || index.column() >= _columns.size()){
        return QVariant();
    }
    QVariant value;
    const cellData &cell = modelData[index.row()][index.column()];
    switch(role)
    {
    case Qt::EditRole:
    {
        value=cell.edit;
        break;
    }
    case Qt::DisplayRole:
    {
        value=cell.display;
        break;
    }
    case Qt::BackgroundRole:
    {
        if (cell.background.isValid()) {
            value = cell.background;
        } else {
            value = QVariant();
        }
        break;
    }
    case Qt::ToolTipRole:
    {
        value=cell.tooltip;
        break;
    }
    case Qt::TextAlignmentRole:
    {
        QMetaType::Type colType = columnType(index.column());
        value=(colType==QMetaType::Int || colType==QMetaType::Double || colType==QMetaType::LongLong)? int(Qt::AlignRight | Qt::AlignVCenter) : int(Qt::AlignLeft | Qt::AlignVCenter);
        break;
    }
    case Qt::CheckStateRole:
    {
        if (columnInfo(index.column()).checkable){
            value = cell.edit.toBool() ? Qt::Checked : Qt::Unchecked;
        } else {
            value = QVariant();
        }
        break;
    }
    default:
        value=QVariant();
        break;
    }
    return value;
}

int RestRoTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return modelData.size();
}

int RestRoTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return _columns.size();
}

QVariant RestRoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Horizontal && role==Qt::DisplayRole && section>=0 && section<columnCount()){
        return colMap.value(_columns.at(section)).snam;
    }
    if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return QString::number(section+1);
    }
    return QAbstractTableModel::headerData(section,orientation,role);
}

colInfo RestRoTableModel::columnInfo(int col) const
{
    return (col>=0 && col<columnCount()) ? colMap.value(_columns.at(col)) : colInfo();
}

QMetaType::Type RestRoTableModel::columnType(int col) const
{
    if (col < 0 || col >= _columns.size()) return QMetaType::QString;
    return RestTableModel::getMetaType(colMap.value(_columns.at(col)).udt_name);
}

void RestRoTableModel::setPath(QString p)
{
    _path=p;
}

void RestRoTableModel::setModelData(const QJsonObject &data)
{
    beginResetModel();
    _columns.clear();
    colMap.clear();
    _title=data.value("title").toString();
    const QJsonArray fields = data.value("fields").toArray();
    for (const QJsonValue &value : fields) {
        colInfo inf;
        inf.nam=value.toObject().value("nam").toString();
        inf.col=value.toObject().value("nam").toString();
        inf.snam=value.toObject().value("snam").toString();
        inf.udt_name=value.toObject().value("udt_name").toString();
        inf.is_pk=false;
        inf.editable=false;
        inf.checkable=false;
        inf.dec=value.toObject().value("dec").toInt();
        inf.relnam="";
        inf.flags=(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        inf.defaultVal.val=QVariant();
        inf.width=value.toObject().value("width").toInt();
        _columns.push_back(inf.nam);
        colMap.insert(inf.nam,inf);
    }
    modelData.clear();
    const QJsonArray rows=data.value("rows").toArray();
    for (const QJsonValue &val : rows) {
        QVector<cellData> row;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        for (const QString &col_nam : qAsConst(_columns)){
#else
        for (const QString &col_nam : std::as_const(_columns)){
#endif
            colInfo col = colMap.value(col_nam);
            QJsonObject obj = val.toObject().value(col.nam).toObject();
            cellData cell;
            cell.display=obj.value("display_role").toString();
            cell.edit=RestTableModel::loadEdtVal(obj.value("edit_role"),col.udt_name);
            QString bg = obj.value("background_role").toString();
            cell.background = bg.isEmpty() ? QColor() : QColor(bg);
            cell.tooltip=obj.value("tooltip_role").toString();
            row.push_back(cell);
        }
        modelData.push_back(row);
    }
    endResetModel();
    emit sigRefresh();
}

bool RestRoTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation==Qt::Horizontal && role==Qt::EditRole && section>=0 && section<columnCount()){
        colMap[_columns.at(section)].snam=value.toString();
        return true;
    }
    return QAbstractTableModel::setHeaderData(section,orientation,value,role);
}

QString RestRoTableModel::title() const
{
    return _title;
}

QString RestRoTableModel::path() const
{
    return _path;
}

QVariant RestRoTableModel::getModelData(int row, QString col) const
{
    return this->data(this->index(row,_columns.indexOf(col)),Qt::EditRole);
}

void RestRoTableModel::select()
{
    if (_path.isEmpty()){
        return;
    }
    QUrl url = QUrl(RestConnection::instance()->getUrl() + "/" + _path);

    queue.enqueue(url);

    // Если прямо сейчас ничего не выполняется — запускаем
    if (!isProcessing) {
        processNextRequest();
    }
}

void RestRoTableModel::selectSync()
{
    QByteArray data;
    bool ok=RestConnection::instance()->sendSyncGet(_path,data);
    if (ok){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        setModelData(doc.object());
    } else {
        clear();
    }
}

void RestRoTableModel::clear()
{
    beginResetModel();
    _columns.clear();
    colMap.clear();
    modelData.clear();
    endResetModel();
    emit sigRefresh();
}

void RestRoTableModel::processNextRequest()
{
    if (queue.isEmpty()) {
        isProcessing = false;
        return;
    }

    isProcessing = true;
    QUrl url = queue.dequeue();

    QNetworkReply *reply = RestConnection::instance()->sendGet(url);

    connect(reply, &QNetworkReply::finished, this, &RestRoTableModel::onResult);

    emit sigStartRefresh();
}

void RestRoTableModel::onResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    // Извлекаем данные сразу. Нам они понадобятся и для ошибок, и для успешного JSON
    const QByteArray data = reply->readAll();
    const QNetworkReply::NetworkError netError = reply->error();

    // Освобождаем память запроса в цикле событий Qt сразу,
    // чтобы не забыть сделать это в многочисленных return
    reply->deleteLater();

    // 1. Обработка штатной отмены запроса
    if (netError == QNetworkReply::OperationCanceledError) {
        processNextRequest();
        return;
    }

    // 2. Обработка сетевых ошибок (404, 500, таймаут и т.д.)
    if (netError != QNetworkReply::NoError) {
        clear();
        QMessageBox::critical(QApplication::activeWindow(), tr("Ошибка сети"), reply->errorString() + "\n" + data, QMessageBox::Cancel);
        processNextRequest(); // Переходим к следующему ПОСЛЕ закрытия диалога
        return;
    }

    // 3. Парсинг JSON-документа
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);

    // 4. Обработка синтаксических ошибок JSON
    if (jsonError.error != QJsonParseError::NoError) {
        clear();
        QString errorString = tr("Ошибка JSON: ") + jsonError.errorString() +
                              tr("\nПозиция: ") + QString::number(jsonError.offset);
        QMessageBox::critical(QApplication::activeWindow(), tr("Ошибка данных"), errorString, QMessageBox::Cancel);
        processNextRequest();
        return;
    }

    // 5. Успешное выполнение: обновляем модель данными
    setModelData(doc.object());

    // Запускаем следующий запрос из очереди
    processNextRequest();
}

