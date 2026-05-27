 #include "resttablemodel.h"

RestTableModel::RestTableModel(QString name, QObject *parent) : QAbstractTableModel(parent), _rname(name)
{
    isProcessing=false;
    _path = "api/autorest/tables/"+_rname;
    block=false;
    insertable=true;
    editor = new DataEditor(&modelData,this);
    loadInfo();
}

Qt::ItemFlags RestTableModel::flags(const QModelIndex &index) const
{
    return colMap.value(_columns.at(index.column())).flags;
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
        value=((colType==QMetaType::Int || colType==QMetaType::Double || colType==QMetaType::LongLong) && !isColumnRel(index.column()))? int(Qt::AlignRight | Qt::AlignVCenter) : int(Qt::AlignLeft | Qt::AlignVCenter);
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

bool RestTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!(this->flags(index) & Qt::ItemIsEditable) || !index.isValid()) {
        return false;
    }
    cellData setVal=modelData[index.row()][index.column()];
    if (role==Qt::CheckStateRole){
        if (columnType(index.column())==QMetaType::Bool){
            setVal.edit=value.toBool();
        } else if(columnType(index.column())==QMetaType::Int){
            setVal.edit=value.toBool()? 1 : 0;
        }
        setVal.display=formatVal(value.toBool(),index.column());
    } else if (role==Qt::EditRole){
        setVal.edit=value;
        if (!isColumnRel(index.column())){
            setVal.display=formatVal(value,index.column());
        }
    } else if (role==Qt::DisplayRole){
        setVal.display=value.toString();
    }

    bool ok=editor->edt(index.row(),index.column(),setVal);
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
    return _columns.size();
}

bool RestTableModel::insertRow(int /*row*/, const QModelIndex &/*parent*/)
{
    if (block || !insertable) {
        return false;
    }
    bool ok=false;
    if (!editor->isAdd() && !editor->isEdt()){
        emit sigAboutToBeInsert();
        beginInsertRows(QModelIndex(),rowCount(),rowCount());
        QVector<cellData> row=defaultRow();
        ok=editor->add(rowCount(),row);
        endInsertRows();
    }
    return ok;
}

bool RestTableModel::removeRow(int row, const QModelIndex &parent)
{
    revert();
    if (!rowCount() || row<0 || row>=rowCount() || (editor->isAdd() && rowCount()==1)) {
        return false;
    }
    QString dat;
    for(int i=0; i<columnCount(); i++) {
        if (!dat.isEmpty()){
            dat+=", ";
        }
        dat+=data(this->index(row,i),Qt::DisplayRole).toString();
    }
    int n=QMessageBox::question(nullptr,QString::fromUtf8("Подтвердите удаление"),
                                  QString::fromUtf8("Подтветждаете удаление ")+dat+QString::fromUtf8("?"),QMessageBox::Yes| QMessageBox::No);
    bool ok=false;
    if (n==QMessageBox::Yes) {
        if (apiDelete(row)) {
            beginRemoveRows(parent,row,row);
            modelData.remove(row);
            endRemoveRows();
            ok=true;
        }
    }
    if (ok){
        if (modelData.size()<1) {
            this->insertRow(0);
        }
        emit sigUpd();
    }
    return ok;
}

bool RestTableModel::refreshRow(int row)
{
    //qDebug()<<"refresh row";
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("old_row",QJsonValue(getRowObject(modelData.at(row))));
    obj.insert("new_row",QJsonValue(getRowObject(modelData.at(row))));
    doc.setObject(obj);
    QByteArray body = doc.toJson();
    QByteArray data;
    bool ok = RestConnection::instance()->sendSyncRequest(_path,"PUT",body,data);
    QJsonDocument rowdoc = QJsonDocument::fromJson(data);
    const QJsonArray rows = rowdoc.array();
    if (ok && rows.size()){
        modelData[row] = loadRow(rows.at(0));
        emit dataChanged(this->index(row,0),this->index(row,columnCount()-1));
    }
    return ok;
}

void RestTableModel::setFilter(QString f)
{
    _filter=f;
}

void RestTableModel::setPath(QString p)
{
    _path=p;
}

void RestTableModel::setInsertable(bool b)
{
    insertable=b;
}

void RestTableModel::setDefaultValue(QString column, QVariant value)
{
    if (colMap.contains(column)){
        colMap[column].defaultVal=value;
    }
}

void RestTableModel::setColumnFlags(QString column, Qt::ItemFlags flags)
{
    if (colMap.contains(column)){
        colMap[column].flags=flags;
    }
}

void RestTableModel::setColumns(const QStringList &cols)
{
    beginResetModel();
    editor->revert();
    modelData.clear();
    _columns=cols;
    endResetModel();
}

QVariant RestTableModel::nullValue(const QString &udt_name)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QVariant::Type type = (QVariant::Type) getMetaType(udt_name);
    return QVariant(type);
#else
    return QVariant(QMetaType(getMetaType(udt_name)),nullptr);
#endif
}

QVariant RestTableModel::nullValue(int column) const
{
    return nullValue(colMap.value(_columns.at(column)).udt_name);
}

QVariant RestTableModel::defaultValue(int column) const
{
    if (column>=0 && column<columnCount()){
        return colMap.value(_columns.at(column)).defaultVal;
    } else {
        return QVariant();
    }
}

QVariant RestTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Horizontal && role==Qt::DisplayRole && section>=0 && section<columnCount()){
        return colMap.value(_columns.at(section)).snam;
    }
    if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        if (editor->isEdt() && section==editor->currentPos()) {
            return QString("|");
        }
        return (editor->isAdd() && (section==editor->currentPos()))? QString("*"): QString("  ");
    }
    return QAbstractTableModel::headerData(section,orientation,role);
}

bool RestTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation==Qt::Horizontal && role==Qt::EditRole && section>=0 && section<columnCount()){
        colMap[_columns.at(section)].snam=value.toString();
        return true;
    }
    return QAbstractTableModel::setHeaderData(section,orientation,value,role);
}

QMetaType::Type RestTableModel::columnType(int col) const
{
    return getMetaType(colMap.value(_columns.at(col)).udt_name);
}

QString RestTableModel::tableName() const
{
    return _tablename;
}

colInfo RestTableModel::columnInfo(int col) const
{
    return (col>=0 && col<columnCount()) ? colMap.value(_columns.at(col)) : colInfo();
}

bool RestTableModel::isColumnRel(int col) const
{
    return (col>=0 && col<columnCount()) ? (!colMap.value(_columns.at(col)).relnam.isEmpty()) : false;
}

bool RestTableModel::isAdd() const
{
    return editor->isAdd();
}

bool RestTableModel::isEdt() const
{
    return editor->isEdt();
}

bool RestTableModel::isEmpty() const
{
    return (rowCount()==1 && isAdd()) || (rowCount()<1);
}

bool RestTableModel::isInsertable() const
{
    return insertable;
}

int RestTableModel::currentEdtRow() const
{
    return editor->currentPos();
}

int RestTableModel::columnIndex(QString nam) const
{
    return _columns.indexOf(nam);
}

void RestTableModel::select()
{
    QUrl url = QUrl(RestConnection::instance()->getUrl()+"/"+_path);
    QUrlQuery query;
    query.addQueryItem("filter",_filter);
    url.setQuery(query);

    queue.enqueue(url);
    if (!isProcessing) {
        processNextRequest();
    }
}

void RestTableModel::processNextRequest()
{
    if (queue.isEmpty()) {
        isProcessing = false;
        return;
    }

    isProcessing = true;
    QUrl url = queue.dequeue();

    QNetworkReply *reply = RestConnection::instance()->sendGet(url);
    connect(reply,SIGNAL(finished()),this,SLOT(onResult()));
}

void RestTableModel::onResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply){
        return;
    }
    if (reply->error()){
        beginResetModel();
        editor->revert();
        modelData.clear();
        editor->add(0,defaultRow());
        endResetModel();
        QMessageBox::critical(nullptr,tr("Ошибка"),reply->errorString()+"\n"+reply->readAll(),QMessageBox::Cancel);
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray rows=doc.array();
        beginResetModel();
        editor->revert();
        modelData.clear();
        for (const QJsonValue &value : rows) {
            modelData.push_back(loadRow(value));
        }
        if (!modelData.size()){
            editor->add(0,defaultRow());
        }
        endResetModel();
    }
    emit sigRefresh();
    reply->deleteLater();
    processNextRequest();
}


void RestTableModel::revert()
{
    block=true;
    int r=editor->currentPos();
    if (editor->isAdd() && rowCount()>1){
        beginRemoveRows(QModelIndex(),r,r);
        editor->revert();
        endRemoveRows();
    } else if (editor->isEdt()){
        editor->revert();
        emit dataChanged(this->index(r,0),this->index(r,columnCount()-1));
        emit headerDataChanged(Qt::Vertical,r,r);
    }
    block=false;
}

bool RestTableModel::submitRow()
{
    if (block) {
        return false;
    }
    if (editor->isEdt()){
        bool ok=false;
        if (editor->isAdd()){
            //qDebug()<<"insert!";
            ok=apiInsert();
        } else {
            //qDebug()<<"update!";
            ok=apiUpdate();
        }
        if (ok) {
            int r=editor->currentPos();
            editor->submit();
            emit dataChanged(this->index(r,0),this->index(r,columnCount()-1));
            emit headerDataChanged(Qt::Vertical,r,r);
            emit sigUpd();
        }
    } else if (editor->isAdd()){
        revert();
    }
    return !(editor->isAdd() || editor->isEdt());
}

void RestTableModel::loadInfo()
{
    QByteArray data;
    bool ok = RestConnection::instance()->sendSyncGet("api/autorest/tableinfo/"+_rname,data);
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
            inf.defaultVal=nullValue(inf.udt_name);
            inf.width = value.toObject().value("width").toVariant();
            _columns.push_back(inf.nam);
            colMap.insert(inf.nam,inf);
        }
    }
}

bool RestTableModel::apiInsert()
{
    QJsonDocument doc;
    doc.setObject(getRowObject(editor->newRow()));
    QByteArray body = doc.toJson();
    QByteArray data;
    //qDebug()<<body;
    bool ok = RestConnection::instance()->sendSyncRequest(_path,"POST",body,data);
    QJsonDocument rowdoc = QJsonDocument::fromJson(data);
    const QJsonArray rows = rowdoc.array();
    if (ok && rows.size()){
        modelData[editor->currentPos()] = loadRow(rows.at(0));
    }
    return ok;
}

bool RestTableModel::apiUpdate()
{
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("old_row",QJsonValue(getRowObject(editor->oldRow())));
    obj.insert("new_row",QJsonValue(getRowObject(editor->newRow())));
    doc.setObject(obj);
    QByteArray body = doc.toJson();
    QByteArray data;
    bool ok = RestConnection::instance()->sendSyncRequest(_path,"PUT",body,data);
    QJsonDocument rowdoc = QJsonDocument::fromJson(data);
    const QJsonArray rows = rowdoc.array();
    if (ok && rows.size()){
        modelData[editor->currentPos()] = loadRow(rows.at(0));
    }
    return ok;
}

bool RestTableModel::apiDelete(int row)
{
    QUrlQuery query;
    for (int i=0; i<columnCount(); i++){
        colInfo inf = colMap.value(_columns.at(i));
        if (inf.is_pk) {
            QVariant val = this->data(this->index(row,i),Qt::EditRole);
            query.addQueryItem(inf.nam,val.toString());
        }
    }
    QByteArray body, data;
    bool ok = RestConnection::instance()->sendSyncRequest(_path+"?"+query.toString(),"DELETE",body,data);
    return ok;
}

QVector<cellData> RestTableModel::loadRow(const QJsonValue &val) const
{
    QVector<cellData> row;
    for (const QString &col_nam : _columns){
        colInfo col = colMap.value(col_nam);
        QJsonObject obj = val.toObject().value(col.nam).toObject();
        cellData cell;
        cell.display=obj.value("display_role").toString();
        cell.edit=loadEdtVal(obj.value("edit_role"),col.udt_name);
        cell.background=QColor(obj.value("background_role").toString());
        cell.tooltip=obj.value("tooltip_role").toString();
        row.push_back(cell);
    }
    return row;
}

QVector<cellData> RestTableModel::defaultRow() const
{
    QVector<cellData> tmpRow;
    for (int i=0; i<columnCount();i++){
        cellData d;
        d.edit=colMap.value(_columns.at(i)).defaultVal;
        d.display=formatVal(d.edit,i);
        d.background=QColor(255,255,255);
        tmpRow.push_back(d);
    }
    return tmpRow;
}

QMetaType::Type RestTableModel::getMetaType(const QString &udt_name)
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

QVariant RestTableModel::loadEdtVal(const QJsonValue &val, const QString &udt_name)
{
    QMetaType::Type type=getMetaType(udt_name);
    if (val.isNull()){
        return nullValue(udt_name);
    }
    switch (type) {
    case QMetaType::Bool: {
        return val.toBool();
    }
    case QMetaType::LongLong: {
        return val.toVariant().toLongLong();
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

QString RestTableModel::formatVal(const QVariant &val, int column) const
{
    QMetaType::Type type=columnType(column);
    if (val.isNull()){
        return QString("");
    }
    if (isColumnRel(column)){
        QByteArray data;
        QUrlQuery query;
        query.addQueryItem("key",val.toString());
        bool ok = RestConnection::instance()->sendSyncGet("api/autorest/relations/"+columnInfo(column).relnam+"?"+query.toString(),data);
        if (ok){
            QJsonDocument doc = QJsonDocument::fromJson(data);
            const QJsonArray arr=doc.array();
            if (arr.size()){
                return arr.at(0).toObject().value("disp").toVariant().toString();
            }
            return QString("");
        }
    }
    switch (type) {
    case QMetaType::Bool: {
        return val.toBool()? tr("Да") : tr("Нет");
    }
    case QMetaType::LongLong: {
        return QLocale().toString(val.toLongLong());
    }
    case QMetaType::Int: {
        return columnInfo(column).checkable ? (val.toBool()? tr("Да") : tr("Нет")) : QLocale().toString(val.toInt());
    }
    case QMetaType::Double: {
        return QLocale().toString(val.toDouble(),'f',columnInfo(column).dec);
    }
    case QMetaType::QDate: {
        return val.toDate().toString("dd.MM.yyyy");
    }
    case QMetaType::QTime: {
        return val.toTime().toString("hh:mm:ss");
    }
    case QMetaType::QDateTime: {
        return val.toDateTime().toString("dd.MM.yyyy, hh:mm");
    }
    default: {
        return val.toString();
    }
    }
    return val.toString();
}

QJsonValue RestTableModel::getJsonValue(const QVariant &val)
{
    if (val.isNull()){
        return QJsonValue();
    }
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    int type=val.type();
#else
    int type=val.typeId();
#endif
    switch (type) {
    case QMetaType::Bool: {
        return QJsonValue(val.toBool());
    }
    case QMetaType::LongLong: {
        return QJsonValue(val.toLongLong());
    }
    case QMetaType::Int: {
        return QJsonValue(val.toInt());
    }
    case QMetaType::Double: {
        return QJsonValue(val.toDouble());
    }
    case QMetaType::QDate: {
        return QJsonValue(val.toDate().toString("yyyy-MM-dd"));
    }
    case QMetaType::QTime: {
        return QJsonValue(val.toTime().toString("hh:mm:ss"));
    }
    case QMetaType::QDateTime: {
        return QJsonValue(val.toDateTime().toString("yyyy-MM-ddThh:mm:ssZ"));
    }
    default: {
        return val.toJsonValue();
    }
    }
    return val.toJsonValue();
}

QJsonObject RestTableModel::getRowObject(const QVector<cellData> &row)
{
    QJsonObject obj;
    for (int i=0; i<_columns.size(); i++){
        cellData cell = row.at(i);
        obj.insert(_columns.at(i),getJsonValue(cell.edit));
    }
    return obj;
}

DataEditor::DataEditor(QVector<QVector<cellData> > *data, QObject *parent) : QObject(parent), mData(data)
{
    pos=-1;
    addFlag=false;
    edtFlag=false;
}

bool DataEditor::add(int p, const QVector<cellData> &row)
{
    bool ok=false;
    if (!addFlag && !edtFlag){
        mData->insert(p,row);
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
    (*mData)[row][col]=val;
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
    if ((edtFlag && !addFlag) || (addFlag && mData->size()==1 && edtFlag)){
        (*mData)[pos]=saveRow;
        edtFlag=false;
    } else if (addFlag && mData->size()>1){
        mData->remove(pos);
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
        r=mData->at(pos);
    }
    return r;
}
