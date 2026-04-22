#include "resttableview.h"

RestTableView::RestTableView(QWidget *parent) : QTableView(parent)
{
    menuEnabled=false;
    writeOk=true;
    setSelectionMode(QAbstractItemView::SingleSelection);
    verticalHeader()->setDefaultSectionSize(verticalHeader()->fontMetrics().height()*1.5);
    verticalHeader()->setFixedWidth(verticalHeader()->fontMetrics().height()*1.2);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    setAutoScroll(true);
    this->setItemDelegate(new RestItemDelegate(this));
    updAct = new QAction(QString::fromUtf8("Обновить"),this);
    removeAct = new QAction(QString::fromUtf8("Удалить"),this);

    connect(updAct,SIGNAL(triggered(bool)),this,SLOT(upd()));
    connect(removeAct,SIGNAL(triggered(bool)),this,SLOT(remove()));
}

void RestTableView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    RestTableModel *restModel = qobject_cast<RestTableModel *>(this->model());
    if (restModel){
        connect(this->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(submit(QModelIndex,QModelIndex)));
        setMenuEnabled(true);
    }
}

void RestTableView::keyPressEvent(QKeyEvent *e)
{
    RestTableModel *restModel = qobject_cast<RestTableModel *>(this->model());
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
        menu.addSeparator();
        if (this->selectionModel()){
            if (this->indexAt(event->pos()).isValid() && this->editTriggers()!=QAbstractItemView::NoEditTriggers){
                menu.addAction(removeAct);
                menu.addSeparator();
            }
        }
        menu.exec(event->globalPos());
    }
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
        setColumnWidth(i,max+22);
    }
}

void RestTableView::setMenuEnabled(bool value)
{
    menuEnabled=value;
}

void RestTableView::upd()
{
    RestTableModel *restModel = qobject_cast<RestTableModel *>(this->model());
    if (restModel) {
        restModel->select();
    }
}

void RestTableView::remove()
{
    RestTableModel *restModel = qobject_cast<RestTableModel *>(this->model());
    QModelIndex ind=this->currentIndex();
    if (restModel && restModel->rowCount() && restModel->removeRow(ind.row())){
        if (ind.row()>0){
            setCurrentIndex(model()->index(ind.row()-1,ind.column()));
        } else if (restModel->rowCount()){
            setCurrentIndex(model()->index(ind.row(),ind.column()));
        }
    }
}

void RestTableView::submit(QModelIndex /*ind*/, QModelIndex oldInd)
{
    if (this->editTriggers()==QAbstractItemView::NoEditTriggers) {
        return;
    }
    RestTableModel *restModel = qobject_cast<RestTableModel *>(this->model());
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
    if (this->editTriggers()!=QAbstractItemView::NoEditTriggers && event->reason()==Qt::MouseFocusReason){
        RestTableModel *restModel = qobject_cast<RestTableModel *>(this->model());
        if (restModel && restModel->isAdd() && !restModel->isEdt()){
            restModel->revert();
        }
    }
    return QTableView::focusOutEvent(event);
}
