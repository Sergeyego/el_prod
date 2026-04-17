#include "resttablemodel.h"

RestTableModel::RestTableModel(QString name, QObject *parent) : QAbstractTableModel(parent), _rname(name)
{
    _path="api/elrtr/parti";
    loadInfo();
}

Qt::ItemFlags RestTableModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsEditable | Qt::ItemIsSelectable |Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

QVariant RestTableModel::data(const QModelIndex &index, int role) const
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
        value=(colType==QMetaType::Int || colType==QMetaType::Double)? int(Qt::AlignRight | Qt::AlignVCenter) : int(Qt::AlignLeft | Qt::AlignVCenter);
        break;
    }
    default:
        value=QVariant();
        break;
    }
    return value;
}

int RestTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return modelData.size();
}

int RestTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return colData.size();
}

QVariant RestTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Horizontal && role==Qt::DisplayRole){
        return colData.at(section).snam;
    }
    return QAbstractTableModel::headerData(section,orientation,role);
}

QMetaType::Type RestTableModel::columnType(int col) const
{
    return getMetaType(colData.at(col).udt_name);
}


void RestTableModel::select()
{
    QByteArray data;
    bool ok = HttpSyncManager::sendGet("api/elrtr/parti",data);
    if (ok) {
        beginResetModel();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray rows=doc.array();
        for (const QJsonValue &value : rows) {
            QVector<cellData> row;
            for (const colInfo &col : qAsConst(colData)){
                QJsonObject obj = value.toObject().value(col.nam).toObject();
                cellData cell;
                cell.display=obj.value("display_role").toString();
                cell.edit=obj.value("edit_role").toVariant();
                cell.background=QColor(obj.value("background_role").toString());
                cell.tooltip=obj.value("tooltip_role").toString();
                row.push_back(cell);
            }
            modelData.push_back(row);
        }
        endResetModel();
    }
}

void RestTableModel::loadInfo()
{
    QByteArray data;
    bool ok = HttpSyncManager::sendGet("api/autorest/tableinfo/"+_rname,data);
    if (ok){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        _tablename=doc.object().value("tablename").toString();
        const QJsonArray cols=doc.object().value("columns").toArray();
        for (const QJsonValue &value : cols) {
            colInfo inf;
            inf.nam=value.toObject().value("nam").toString();
            inf.col=value.toObject().value("col").toString();
            inf.snam=value.toObject().value("snam").toString();
            inf.udt_name=value.toObject().value("udt_name").toString();
            inf.is_pk=value.toObject().value("is_pk").toBool();
            inf.editale=value.toObject().value("editable").toBool();
            inf.checkable=value.toObject().value("checkable").toBool();
            inf.dec=value.toObject().value("dec").toInt();
            inf.relnam=value.toObject().value("relnam").toString();
            colData.push_back(inf);
        }
    }
}

QMetaType::Type RestTableModel::getMetaType(const QString &udt_name) const
{
    QMetaType::Type type;
    if (udt_name=="int2" || udt_name=="int4" || udt_name=="int8"){
        type=QMetaType::Int;
    } else if (udt_name=="float4" || udt_name=="float8" || udt_name=="numeric"){
        type=QMetaType::Double;
    } else if (udt_name=="date"){
        type=QMetaType::QDate;
    } else if (udt_name=="bool"){
        type=QMetaType::Bool;
    } else {
        type=QMetaType::QString;
    }
    return type;
}

QVariant RestTableModel::loadEdtVal(const QJsonValue &val, const QString &udt_name) const
{
    QMetaType::Type type=getMetaType(udt_name);
    return QVariant();
}
