 #include "resttablemodel.h"

RestTableModel::RestTableModel(QString name, QObject *parent) : QAbstractTableModel(parent), _rname(name)
{
    _path = "api/elrtr/parti";
    editor = new DataEditor(&modelData,this);
    loadInfo();
}

Qt::ItemFlags RestTableModel::flags(const QModelIndex &index) const
{
    return colData.at(index.column()).flags;
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
        value=((colType==QMetaType::Int || colType==QMetaType::Double || colType==QMetaType::LongLong) && colData.at(index.column()).relnam.isEmpty())? int(Qt::AlignRight | Qt::AlignVCenter) : int(Qt::AlignLeft | Qt::AlignVCenter);
        break;
    }
    case Qt::CheckStateRole:
    {
        if (colData.at(index.column()).checkable){
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

bool RestTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!(this->flags(index) & Qt::ItemIsEditable)) return false;
    cellData setVal=modelData[index.row()][index.column()];

    if (columnType(index.column())==QMetaType::Bool){
        setVal.edit=value.toBool();
    } else if(!data(index,Qt::CheckStateRole).isNull() && columnType(index.column())==QMetaType::Int){
        setVal.edit=value.toBool()? 1 : 0;
    } else {
        if (role==Qt::DisplayRole){
            setVal.display=value.toString();
        } else if (role==Qt::EditRole){
            setVal.edit=value;
        }
    }

    bool ok=false;
    ok=editor->edt(index.row(),index.column(),setVal);
    emit dataChanged(index,index);
    emit headerDataChanged(Qt::Vertical,index.row(),index.row());
    return ok;
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

colInfo RestTableModel::columnInfo(int col) const
{
    return colData.at(col);
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
                cell.edit=loadEdtVal(obj.value("edit_role"),col.udt_name);
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
            inf.flags = inf.editale ? (Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled) : (Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            colData.push_back(inf);
        }
    }
}

QMetaType::Type RestTableModel::getMetaType(const QString &udt_name) const
{
    QStringList boolList = {"bool"};
    QStringList longList = {"int8"};
    QStringList intList = {"int2", "int4", "oid", "regproc", "xid"};
    QStringList doubleList = {"float4", "float8", "numeric"};
    QStringList dateList = {"date"};
    QStringList timeList = {"time", "timetz"};
    QStringList dateTimeList = {"timestamp", "timestamptz"};
    QStringList byteaList = {"bytea"};

    QMetaType::Type type = QMetaType::UnknownType;
    if (boolList.contains(udt_name)){
        type=QMetaType::Bool;
    } else if (longList.contains(udt_name)){
        type=QMetaType::LongLong;
    } else if (intList.contains(udt_name)){
        type=QMetaType::Int;
    } else if (doubleList.contains(udt_name)){
        type=QMetaType::Double;
    } else if (dateList.contains(udt_name)){
        type=QMetaType::QDate;
    } else if (timeList.contains(udt_name)){
        type=QMetaType::QTime;
    } else if (dateTimeList.contains(udt_name)){
        type=QMetaType::QDateTime;
    } else if (byteaList.contains(udt_name)){
        type=QMetaType::QByteArray;
    } else {
        type=QMetaType::QString;
    }
    return type;
}

QVariant RestTableModel::loadEdtVal(const QJsonValue &val, const QString &udt_name) const
{
    QMetaType::Type type=getMetaType(udt_name);
    if (val.isNull()){
        return QVariant(QMetaType(type),nullptr);
    }
    switch (type) {
    case QMetaType::Bool: {
        return val.toBool();
    }
    case QMetaType::LongLong: {
        return val.toInteger();
    }
    case QMetaType::Int: {
        return val.toInt();
    }
    case QMetaType::Double: {
        if (udt_name=="numeric"){
            return val.toString().toDouble();
        } else {
            return val.toDouble();
        }
    }
    case QMetaType::QDate: {
        return QDateTime::fromString(val.toString(),Qt::ISODate).toLocalTime().date();
    }
    case QMetaType::QTime: {
        return QTime::fromString(val.toString(),Qt::ISODate);
    }
    case QMetaType::QDateTime: {
        return QDateTime::fromString(val.toString(),Qt::ISODate).toLocalTime();
    }
    case QMetaType::QByteArray: {
        return val.toVariant();
    }
    case QMetaType::QString: {
        return val.toString();
    }
    default: {
        return val.toVariant();
    }
    }
    return QVariant();
}

DataEditor::DataEditor(QVector<QVector<cellData> > *data, QObject *parent) : QObject(parent), mData(data)
{
    pos=-1;
    addFlag=false;
    edtFlag=false;
}

bool DataEditor::add(int p, QVector<cellData> &row)
{
    bool ok=false;
    if (!addFlag && !edtFlag){
        //mData->insertRow(p,row);
        pos=p;
        addFlag=true;
        ok=true;
    }
    return ok;
}

bool DataEditor::edt(int row, int col, cellData val)
{
    if((edtFlag && row!=pos) || (addFlag && row!=pos)) return false;
    if (!edtFlag){
        pos=row;
        saveRow=mData->at(row);
    }

    //mData[row][col]=val;
    edtFlag=true;
    return true;
}

void DataEditor::submit()
{
    addFlag=false;
    edtFlag=false;
    pos=-1;
}

void DataEditor::revert()
{
    if ((edtFlag && !addFlag) || (addFlag && /*mData->rowCount()==1 && */edtFlag)){
        //mData->setRow(saveRow,pos);
        edtFlag=false;
    } else if (addFlag /*&& mData->rowCount()>1*/){
        //mData->delRow(pos);
        addFlag=false;
        edtFlag=false;
    } else {
        edtFlag=false;
        addFlag=false;
        pos=-1;
    }
}

bool DataEditor::isAdd()
{
    return addFlag;
}

bool DataEditor::isEdt()
{
    return edtFlag;
}

int DataEditor::currentPos()
{
    return pos;
}

QVector<cellData> DataEditor::oldRow()
{
    return saveRow;
}

QVector<cellData> DataEditor::newRow()
{
    QVector<cellData> r;
    if (addFlag || edtFlag){
        /*for (int i=0; i<mData->columnCount(); i++){
            r.push_back(mData->column(i)->data.at(pos));
        }*/
    }
    return r;
}
