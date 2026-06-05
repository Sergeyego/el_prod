#include "restitemdelegate.h"

RestItemDelegate::RestItemDelegate(QObject *parent) : QItemDelegate(parent)
{

}

QWidget* RestItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const RestTableModel *restTableModel = qobject_cast<const RestTableModel *>(index.model());
    if (!restTableModel) {
        return QItemDelegate::createEditor(parent, option, index);
    }
    QWidget *editor = nullptr;
    if (restTableModel->isColumnRel(index.column())){
        editor = new RestComboBox(parent);
    } else {
        switch (restTableModel->columnType(index.column())){
        case QMetaType::Bool:
        {
            editor = nullptr;
            break;
        }
        case QMetaType::QString:
        {
            editor = new QLineEdit(parent);
            break;
        }
        case QMetaType::Int:
        {
            editor = restTableModel->data(index,Qt::CheckStateRole).isNull() ? new QLineEdit(parent) : nullptr;
            break;
        }
        case QMetaType::LongLong:
        {
            editor = new QLineEdit(parent);
            break;
        }
        case QMetaType::Double:
        {
            editor = new QLineEdit(parent);
            break;
        }
        case QMetaType::QDate:
        {
            editor = new RestDateEdit(parent);
            break;
        }
        case QMetaType::QDateTime:
        {
            editor = new RestDateTimeEdit(parent);
            break;
        }
        case QMetaType::QTime:
        {
            QTimeEdit *te = new QTimeEdit(parent);
            te->setDisplayFormat("hh:mm:ss");
            editor = te;
            break;
        }
        default:
        {
            editor=QItemDelegate::createEditor(parent, option, index);
            break;
        }
        }
    }
    if (editor) {
        editor->installEventFilter(const_cast<RestItemDelegate *>(this));
        emit createEdt(index);
    }
    return editor;
}

void RestItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const RestTableModel *restTableModel = qobject_cast<const RestTableModel *>(index.model());
    if (restTableModel){
        if (restTableModel->isColumnRel(index.column())){
            RestComboBox *combo = qobject_cast<RestComboBox *>(editor);
            if (combo) {
                if (combo->model()!=RelModels::instance()->getModel(restTableModel->columnInfo(index.column()).relnam)){
                    connect(combo,SIGNAL(sigActionEdtRel(QModelIndex)),this,SLOT(edtRels(QModelIndex)));
                }
                combo->setIndex(index);
                return;
            } else {
                QLineEdit *le = qobject_cast<QLineEdit *>(editor);
                if (le){
                    le->setText(index.model()->data(index,Qt::DisplayRole).toString());
                    return;
                }
            }
        }
        QVariant dat=restTableModel->data(index,Qt::EditRole);
        switch (restTableModel->columnType(index.column())){
        case QMetaType::Int:
        case QMetaType::LongLong:
        {
            QLineEdit *line = qobject_cast<QLineEdit *>(editor);
            if (line){
                if (!line->validator()){
                    QIntValidator *v = new QIntValidator(line);
                    v->setLocale(QLocale::English);
                    line->setValidator(v);
                }
                if (dat.isNull()){
                    line->clear();
                } else {
                    line->setText(dat.toString());
                }
                return;
            }
            break;
        }
        case QMetaType::Double:
        {
            QLineEdit *line = qobject_cast<QLineEdit *>(editor);
            if (line){
                if (!line->validator()){
                    QDoubleValidator *v = new QDoubleValidator(line);
                    v->setLocale(QLocale::English);
                    v->setDecimals(restTableModel->columnInfo(index.column()).dec);
                    line->setValidator(v);
                }
                if (dat.isNull()){
                    line->clear();
                } else {
                    line->setText(QString::number(dat.toDouble(),'f',restTableModel->columnInfo(index.column()).dec));
                }
                return;
            }
            break;
        }
        case QMetaType::QDate:
        {
            RestDateEdit *dateEdit = qobject_cast<RestDateEdit *>(editor);
            if (dateEdit){
                if (dat.isNull()){
                    dateEdit->setDate(dateEdit->minimumDate());
                } else {
                    dateEdit->setDate(dat.toDate());
                }
                return;
            }
            break;
        }
        case QMetaType::QDateTime:
        {
            RestDateTimeEdit *dateTimeEdit = qobject_cast<RestDateTimeEdit *>(editor);
            if (dateTimeEdit){
                if (dat.isNull()){
                    dateTimeEdit->setDateTime(dateTimeEdit->minimumDateTime());
                } else {
                    dateTimeEdit->setDateTime(dat.toDateTime());
                }
                return;
            }
            break;
        }
        default:
        {
            QItemDelegate::setEditorData(editor, index);
            return;
        }
        }
    }
    return QItemDelegate::setEditorData(editor, index);
}

void RestItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!index.isValid()){
        return;
    }
    RestTableModel *restTableModel = qobject_cast<RestTableModel *>(model);
    if (restTableModel){
        const QMetaType::Type type = restTableModel->columnType(index.column());
        if (restTableModel->isColumnRel(index.column())){
            RestComboBox *combo = qobject_cast<RestComboBox *>(editor);
            if (combo) {
                colVal data=combo->getCurrentData();
                if (combo->currentText().isEmpty()){
                    data.val=restTableModel->nullValue(index.column());
                    data.disp=QString();
                }
                restTableModel->setData(index,data.val,Qt::EditRole);
                restTableModel->setData(index,data.disp,Qt::DisplayRole);
                return;
            }
        } else {
            QLineEdit *le = qobject_cast<QLineEdit *>(editor);
            if (le){
                if (le->text().isEmpty()) {
                    restTableModel->setData(index,restTableModel->nullValue(index.column()),Qt::EditRole);
                    return;
                } else {
                    if (type==QMetaType::Int){
                        restTableModel->setData(index,le->text().toInt(),Qt::EditRole);
                    } else if (type==QMetaType::LongLong){
                        restTableModel->setData(index,le->text().toLongLong(),Qt::EditRole);
                    } else if (type==QMetaType::Double){
                        restTableModel->setData(index,le->text().toDouble(),Qt::EditRole);
                    } else {
                        restTableModel->setData(index,le->text(),Qt::EditRole);
                    }
                    return;
                }
            }
            if (type==QMetaType::QDate){
                RestDateEdit *dateEdit = qobject_cast<RestDateEdit *>(editor);
                if (dateEdit){
                    if (dateEdit->date()==dateEdit->minimumDate()){
                        restTableModel->setData(index,restTableModel->nullValue(index.column()),Qt::EditRole);
                    } else {
                        restTableModel->setData(index,dateEdit->date(),Qt::EditRole);
                    }
                    return;
                }
            }
            if (type==QMetaType::QDateTime){
                RestDateTimeEdit *dateTimeEdit = qobject_cast<RestDateTimeEdit *>(editor);
                if (dateTimeEdit){
                    if (dateTimeEdit->dateTime()==dateTimeEdit->minimumDateTime()){
                        restTableModel->setData(index,restTableModel->nullValue(index.column()),Qt::EditRole);
                    } else {
                        restTableModel->setData(index,dateTimeEdit->dateTime(),Qt::EditRole);
                    }
                    return;
                }
            }
        }
    }
    return QItemDelegate::setModelData(editor,model,index);
}

bool RestItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type()==QEvent::FocusOut){
        QLineEdit *line = qobject_cast<QLineEdit *>(object);
        if (line){
            emit commitData(line);
            emit closeEditor(line);
            return false;
        }
    }
    if (event->type()==QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if(keyEvent->key()==Qt::Key_Tab || keyEvent->key()==Qt::Key_Down || keyEvent->key()==Qt::Key_Up){
            QWidget *editor = qobject_cast<QWidget*>(object);
            emit commitData(editor);
            emit closeEditor(editor);
            return false;
        }

        QLineEdit *line = qobject_cast<QLineEdit *>(object);
        if (line && line->validator()) {
            const QDoubleValidator *val = qobject_cast< const QDoubleValidator *>(line->validator());
            if (val){
                if (keyEvent->text()==",") {
                    line->insert(".");
                    return true;
                }
            }
        }
    }
    return QItemDelegate::eventFilter(object,event);
}

void RestItemDelegate::edtRels(QModelIndex index)
{
    const RestTableModel *restTableModel = qobject_cast<const RestTableModel *>(index.model());
    if (restTableModel && restTableModel->isColumnRel(index.column())){
        colInfo ci = restTableModel->columnInfo(index.column());
        RestTableDialog d(ci.relnam);
        d.setWindowTitle("Редактирование таблицы "+ci.snam);
        d.model->select();
        if (d.exec()==QDialog::Accepted){
            QVariantList pk = d.currentPk();
            RestComboBox *combo = qobject_cast<RestComboBox *>(sender());
            if (combo && pk.size()==1){
                colVal c;
                c.val=pk.at(0);
                combo->setCurrentData(c);
            }
        }
        RelModels::instance()->getModel(ci.relnam)->refresh();
    }
}
