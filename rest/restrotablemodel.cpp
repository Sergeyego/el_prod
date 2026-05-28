#include "restrotablemodel.h"

RestRoTableModel::RestRoTableModel(QObject *parent) : QAbstractTableModel{parent}
{
    isProcessing=false;
}

QVariant RestRoTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()){
        return QVariant();
    }
    QVariant value;
    cellData cell = modelData[index.row()][index.column()];
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
        value=cell.background;
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
        inf.editale=false;
        inf.checkable=false;
        inf.dec=value.toObject().value("dec").toInt();
        inf.relnam="";
        inf.flags=(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        inf.defaultVal=QVariant();
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
            cell.background=QColor(obj.value("background_role").toString());
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
    QUrl url = QUrl(RestConnection::instance()->getUrl()+"/"+_path);

    queue.enqueue(url);
    if (!isProcessing) {
        processNextRequest();
    }
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
    connect(reply,SIGNAL(finished()),this,SLOT(onResult()));
    emit sigStartRefresh();
}

void RestRoTableModel::onResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply){
        return;
    }
    if (reply->error()!=QNetworkReply::NoError){
        beginResetModel();
        _columns.clear();
        colMap.clear();
        endResetModel();
        emit sigRefresh();
        QMessageBox::critical(nullptr,tr("Ошибка"),reply->errorString()+"\n"+reply->readAll(),QMessageBox::Cancel);
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        setModelData(doc.object());
    }
    reply->deleteLater();
    processNextRequest();
}