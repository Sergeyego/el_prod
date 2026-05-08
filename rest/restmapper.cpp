#include "restmapper.h"

RestMapper::RestMapper(QAbstractItemView *v, QWidget *parent) :
    QWidget(parent)
{
    cmdNew = new QPushButton(QString::fromUtf8("Нов."),this);
    cmdWrite = new QPushButton(QString::fromUtf8("Запись"),this);
    cmdEdt = new QPushButton(QString::fromUtf8("Редакт."),this);
    cmdEsc = new QPushButton(QString::fromUtf8("Сброс"),this);
    cmdDel = new QPushButton(QString::fromUtf8("Удал."),this);
    cmdWrite->setEnabled(false);
    cmdEsc->setEnabled(false);
    cmdNew->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    cmdEdt->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
    cmdWrite->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    cmdDel->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    cmdEsc->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    mainLayout = new QHBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(cmdEdt);
    mainLayout->addWidget(cmdNew);
    mainLayout->addWidget(cmdDel);
    mainLayout->addWidget(cmdWrite);
    mainLayout->addWidget(cmdEsc);

    viewer=v;
    viewer->setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewer->setSelectionMode(QAbstractItemView::SingleSelection);
    viewer->setSelectionBehavior(QAbstractItemView::SelectRows);
    addLock(viewer);
    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setModel(v->model());
    RestItemDelegate *delegate = new RestItemDelegate(this);
    setItemDelegate(delegate);

    isEdt=false;
    defaultFocus=0;

    RestTableModel *restModel = qobject_cast<RestTableModel *>(mapper->model());
    if (restModel){
        if (!restModel->isInsertable()){
            cmdNew->hide();
        }
        connect(restModel,SIGNAL(sigRefresh()),this,SLOT(last()));
        connect(restModel,SIGNAL(sigRefresh()),this,SLOT(checkEmpty()));
    }

    connect(cmdNew,SIGNAL(clicked()),this,SLOT(slotNew()));
    connect(cmdWrite,SIGNAL(clicked()),this,SLOT(slotWrite()));
    connect(cmdEdt,SIGNAL(clicked(bool)),this,SIGNAL(sigEdt()));
    connect(cmdEdt,SIGNAL(clicked()),this,SLOT(slotEdt()));
    connect(cmdEsc,SIGNAL(clicked()),this,SLOT(slotEsc()));
    connect(cmdDel,SIGNAL(clicked()),this,SLOT(slotDel()));
    connect(mapper->itemDelegate(),SIGNAL(commitData(QWidget*)),this,SLOT(slotEdt()));
    connect(viewer->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),mapper,SLOT(setCurrentModelIndex(QModelIndex)));
    connect(mapper,SIGNAL(currentIndexChanged(int)),this,SIGNAL(currentIndexChanged(int)));

}

RestMapper::~RestMapper()
{

}

void RestMapper::addLock(QWidget *widget)
{
    lock1.push_back(widget);
}

void RestMapper::addUnLock(QWidget *widget)
{
    lock2.push_back(widget);
}

void RestMapper::addEmptyLock(QWidget *widget)
{
    lockEmpty.push_back(widget);
}

void RestMapper::lock(bool val)
{
    isEdt=val;
    cmdDel->setEnabled(!val);
    cmdEdt->setEnabled(!val);
    cmdEsc->setEnabled(val);
    cmdNew->setEnabled(!val);
    cmdWrite->setEnabled(val);
    for (int i=0; i<lock1.size(); i++) lock1[i]->setEnabled(!val);
    for (int i=0; i<lock2.size(); i++) lock2[i]->setEnabled(val);
    for (int i=0; i<lockEmpty.size(); i++) lockEmpty[i]->setEnabled(!val);
    emit lockChanged(val);
}

void RestMapper::checkEmpty()
{
    RestTableModel *restModel = qobject_cast<RestTableModel *>(mapper->model());
    if (restModel) {
        bool val= restModel->rowCount()<=1 && restModel->isAdd();
        for (int i=0; i<lockEmpty.size(); i++) lockEmpty[i]->setEnabled(!val);
        cmdEdt->setEnabled(!val);
        cmdDel->setEnabled(!val);
        emit lockChanged(val);
    }
}

bool RestMapper::isLock()
{
    return (cmdWrite->isEnabled());
}

void RestMapper::addMapping(QWidget *widget, QString section)
{
    QPalette pal=widget->palette();
    pal.setColor(QPalette::Disabled, QPalette::Text, pal.color(QPalette::Active,QPalette::Text));
    pal.setColor(QPalette::Disabled, QPalette::WindowText, pal.color(QPalette::Active,QPalette::WindowText));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, pal.color(QPalette::Active,QPalette::ButtonText));
    widget->setEnabled(false);
    widget->setPalette(pal);
    addUnLock(widget);
    RestTableModel *restModel = qobject_cast<RestTableModel *>(mapper->model());
    if (restModel) {
        int ind=restModel->columnIndex(section);
        if (ind>=0){
            mapper->addMapping(widget,ind);
        } else {
            qDebug()<<"Not found: "+section;
        }
    }
}

int RestMapper::currentIndex()
{
    return mapper->currentIndex();
}

void RestMapper::setDefaultFocus(int n)
{
    defaultFocus=n;
}

void RestMapper::setItemDelegate(QAbstractItemDelegate *delegate)
{
    mapper->setItemDelegate(delegate);
}

QVariant RestMapper::modelData(int row, QString column)
{
    QVariant value;
    RestTableModel *restModel = qobject_cast<RestTableModel *>(mapper->model());
    if (restModel) {
        int ind=restModel->columnIndex(column);
        if (ind>=0){
            value=restModel->data(restModel->index(row,ind),Qt::EditRole);
        }
    }
    return value;
}

void RestMapper::refresh()
{
    mapper->setCurrentIndex(mapper->currentIndex());
}

void RestMapper::setCurrentViewRow(int row)
{
    int n=0;
    QTableView *table = qobject_cast<QTableView *> (viewer);
    if (table){
        bool hidden=table->isColumnHidden(n);
        while (hidden && n<table->model()->columnCount()){
            n++;
            hidden=table->isColumnHidden(n);
        }
    } else {
        QListView *listview = qobject_cast<QListView *> (viewer);
        if (listview){
            n=listview->modelColumn();
        }
    }
    viewer->setCurrentIndex(viewer->model()->index(row,n));
    viewer->scrollTo(viewer->currentIndex());
}


void RestMapper::slotNew()
{
    RestTableModel *restModel = qobject_cast<RestTableModel *>(mapper->model());
    if (restModel) {
        restModel->insertRow(restModel->rowCount());
        mapper->toLast();
        setCurrentViewRow(restModel->rowCount()-1);
        if (restModel->isAdd()) lock(true);
        if (!defaultFocus){
            for (int i=0; i<mapper->model()->columnCount(); i++){
                if (mapper->mappedWidgetAt(i)) {
                    mapper->mappedWidgetAt(i)->setFocus();
                    break;
                }
            }
        } else {
            if (mapper->mappedWidgetAt(defaultFocus)) mapper->mappedWidgetAt(defaultFocus)->setFocus();
        }
    }
}

void RestMapper::slotEdt()
{
    RestTableModel *restModel = qobject_cast<RestTableModel *>(mapper->model());
    if (restModel) {
        restModel->refreshRow(mapper->currentIndex());
    }
    lock(true);
}

void RestMapper::slotDel()
{
    RestTableModel *restModel = qobject_cast<RestTableModel *>(mapper->model());
    if (restModel) {
        restModel->removeRow(mapper->currentIndex());
    }
    checkEmpty();
}


void RestMapper::slotWrite()
{
    RestTableModel *restModel = qobject_cast<RestTableModel *>(mapper->model());
    if (restModel) {
        this->setFocus();
        mapper->submit();
        bool ok=restModel->submitRow();
        if (ok) {
            lock(false);
        }
        mapper->setCurrentIndex(mapper->currentIndex());
        if (ok){
            emit sigWrite();
        }
    }
}

void RestMapper::slotEsc()
{
    RestTableModel *restModel = qobject_cast<RestTableModel *>(mapper->model());
    this->setFocus();
    if (restModel){
        if (restModel->isAdd()) {
            restModel->revert();
            setCurrentViewRow(restModel->rowCount()-1);
        } else if (restModel->isEdt()){
            restModel->revert();
        }
    }
    lock(false);
    mapper->setCurrentIndex(mapper->currentIndex());
    checkEmpty();
}

void RestMapper::first()
{
    mapper->toFirst();
    setCurrentViewRow(0);
}

void RestMapper::last()
{
    mapper->toLast();
    setCurrentViewRow(mapper->model()->rowCount()-1);
}
