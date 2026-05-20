#include "resttableview.h"

RestTableView::RestTableView(QWidget *parent) : QTableView(parent)
{
    menuEnabled=false;
    writeOk=true;
    restModel=nullptr;
    restRoModel=nullptr;
    setAutoScroll(true);
    verticalHeader()->setDefaultSectionSize(verticalHeader()->fontMetrics().height()*1.5);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    updAct = new QAction(QString::fromUtf8("Обновить"),this);
    removeAct = new QAction(QString::fromUtf8("Удалить"),this);
    xlsxAct = new QAction(QString::fromUtf8("Сохранить в файл"),this);

    connect(updAct,SIGNAL(triggered(bool)),this,SLOT(upd()));
    connect(removeAct,SIGNAL(triggered(bool)),this,SLOT(remove()));
    connect(xlsxAct,SIGNAL(triggered(bool)),this,SLOT(saveXlsx()));
}

void RestTableView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    restModel = qobject_cast<RestTableModel *>(this->model());
    if (restModel){
        verticalHeader()->setFixedWidth(verticalHeader()->fontMetrics().height()*1.2);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setItemDelegate(new RestItemDelegate(this));
        for (int i=0; i<restModel->columnCount(); i++){
            QVariant width = restModel->columnInfo(i).width;
            if (!width.isNull()){
                if (width.toInt()>0){
                    this->setColumnWidth(i,width.toInt());
                } else {
                    this->setColumnHidden(i,true);
                }
            }
        }
        connect(this->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(submit(QModelIndex,QModelIndex)));
        setMenuEnabled(true);
    } else {
        restRoModel = qobject_cast<RestRoTableModel *>(this->model());
        if (restRoModel){
            connect(restRoModel,SIGNAL(sigRefresh()),this,SLOT(resizeToContents()));
            setMenuEnabled(true);
        }
    }
}

void RestTableView::keyPressEvent(QKeyEvent *e)
{
    if (restModel && this->editTriggers()!=QAbstractItemView::NoEditTriggers){
        const int row=currentIndex().row();
        switch (e->key()){
        case Qt::Key_Delete:
        case Qt::Key_D:
        {
            if (e->modifiers()==Qt::ControlModifier) {
                remove();
            }
            QTableView::keyPressEvent(e);
            break;
        }
        case Qt::Key_Escape:
        {
            restModel->revert();
            break;
        }
        case Qt::Key_Down:
        {
            const int rowCount=restModel->rowCount();
            if ((row==rowCount-1) || !rowCount){
                if (restModel->isEdt()){
                    restModel->submitRow();
                }
                restModel->insertRow(restModel->rowCount());
            }
            QTableView::keyPressEvent(e);
            break;
        }
        case Qt::Key_Tab:
        {
            int j=restModel->columnCount()-1;
            bool hidden=isColumnHidden(j);
            while (hidden && j>0){
                j--;
                hidden=this->isColumnHidden(j);
            }
            if ((currentIndex().column()==j) && (row==restModel->rowCount()-1)) {
                if (restModel->isEdt()){
                    if (restModel->submitRow()){
                        restModel->insertRow(restModel->rowCount());
                        QTableView::keyPressEvent(e);
                    }
                } else if (restModel->insertRow(restModel->rowCount())){
                    QTableView::keyPressEvent(e);
                }
            } else {
                QTableView::keyPressEvent(e);
            }
            break;
        }
        default:
        {
            QTableView::keyPressEvent(e);
            break;
        }
        }

    } else {
        QTableView::keyPressEvent(e);
    }
}

void RestTableView::contextMenuEvent(QContextMenuEvent *event)
{
    if (menuEnabled){
        QMenu menu(this);
        menu.addAction(updAct);
        if (restModel || restRoModel){
            menu.addAction(xlsxAct);
        }
        menu.addSeparator();
        if (restModel && this->selectionModel()){
            if (this->indexAt(event->pos()).isValid() && this->editTriggers()!=QAbstractItemView::NoEditTriggers){
                menu.addAction(removeAct);
                menu.addSeparator();
            }
        }
        menu.exec(event->globalPos());
    }
}

int RestTableView::getSpace(int column)
{
    int space=12;
    if (restModel){
        QMetaType::Type type = restModel->columnType(column);
        if (type==QMetaType::QDate || type==QMetaType::QDateTime || type==QMetaType::QTime){
            space+=16;
        } else if (restModel->isColumnRel(column)){
            space+=28;
        }
    }
    return space;
}

void RestTableView::resizeToContents()
{
    if (!model()) return;
    int n=model()->columnCount();
    int m=model()->rowCount();
    int max=0;
    QStringList l;
    QString s;
    for (int i=0; i<n; i++){
        s=model()->headerData(i,Qt::Horizontal).toString();
        l=s.split("\n");
        max=0;
        for (int k=0; k<l.size(); k++){
#if (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
            if (max<fontMetrics().width(l.at(k)))
                max=fontMetrics().width(l.at(k));
#else
            if (max<fontMetrics().horizontalAdvance(l.at(k)))
                max=fontMetrics().horizontalAdvance(l.at(k));
#endif
        }
        for (int j=0; j<m; j++){
            s=model()->data(model()->index(j,i)).toString();
            l=s.split("\n");
            for (int k=0; k<l.size(); k++){
#if (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
                if (max<fontMetrics().width(l.at(k)))
                    max=fontMetrics().width(l.at(k));
#else
                if (max<fontMetrics().horizontalAdvance(l.at(k)))
                    max=fontMetrics().horizontalAdvance(l.at(k));
#endif
            }
        }
        if (max>300) {
            max=300;
        }
        setColumnWidth(i,max+getSpace(i));
    }
}

void RestTableView::setMenuEnabled(bool value)
{
    menuEnabled=value;
}

void RestTableView::saveXlsx()
{
    if (restRoModel){
        QJsonDocument doc;
        QJsonArray arrRow;
        QJsonArray arrColumn;
        for (int j=0; j<restRoModel->columnCount(); j++){
            QJsonObject colObj;
            colInfo inf = restRoModel->columnInfo(j);
            colObj.insert("key",inf.nam);
            colObj.insert("header",inf.snam);
            colObj.insert("id_type",RestTableModel::getMetaType(inf.udt_name));
            colObj.insert("dec",inf.dec);
            colObj.insert("width",this->columnWidth(j));
            arrColumn.push_back(QJsonValue(colObj));
        }
        for (int i=0; i<restRoModel->rowCount(); i++){
            QJsonObject rowObj;
            for (int j=0; j<restRoModel->columnCount(); j++){
                colInfo inf = restRoModel->columnInfo(j);
                QVariant data=restRoModel->data(restRoModel->index(i,j),Qt::EditRole);
                rowObj.insert(inf.nam,RestTableModel::getJsonValue(data));
            }
            arrRow.push_back(QJsonValue(rowObj));
        }
        QJsonObject obj;
        obj.insert("title",restRoModel->title());
        obj.insert("header_height",this->horizontalHeader()->height());
        obj.insert("columns",arrColumn);
        obj.insert("rows",arrRow);
        doc.setObject(obj);
        QByteArray data;
        bool ok = HttpSyncManager::sendRequest("api/xlsx/create","POST",doc.toJson(),data,"application/json");
        if (ok) {
            QString totalName=QDir::homePath()+"/xlsx/test.xlsx";
            QFile file(totalName);
            if (file.open(QIODevice::WriteOnly)){
                file.write(data);
                file.close();
                QFileInfo fileInfo(file);
                QDesktopServices::openUrl((QUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))));
            }
        }
        //qDebug()<<arrColumn;
    }
}

void RestTableView::upd()
{
    if (restModel) {
        restModel->select();
    } else if (restRoModel){
        restRoModel->select();
    }
}

void RestTableView::remove()
{
    if (restModel){
        QModelIndex ind=this->currentIndex();
        if (restModel->rowCount() && restModel->removeRow(ind.row())){
            if (ind.row()>0){
                setCurrentIndex(model()->index(ind.row()-1,ind.column()));
            } else if (restModel->rowCount()){
                setCurrentIndex(model()->index(ind.row(),ind.column()));
            }
        }
    }
}

void RestTableView::submit(QModelIndex /*ind*/, QModelIndex oldInd)
{
    if (this->editTriggers()==QAbstractItemView::NoEditTriggers) {
        return;
    }
    if (restModel){
        if (!writeOk){
            writeOk=true;
            return;
        }
        if (restModel->isEdt() || (restModel->isAdd() && oldInd.row()==restModel->currentEdtRow())){
            writeOk=restModel->submitRow();
        }
    }
}

void RestTableView::focusOutEvent(QFocusEvent *event)
{
    if (restModel && this->editTriggers()!=QAbstractItemView::NoEditTriggers && event->reason()==Qt::MouseFocusReason){
        if (restModel->isAdd() && !restModel->isEdt()){
            restModel->revert();
        }
    }
    return QTableView::focusOutEvent(event);
}
