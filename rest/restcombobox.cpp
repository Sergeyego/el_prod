#include "restcombobox.h"

RestComboBox::RestComboBox(QWidget *parent) : QComboBox(parent)
{
    this->setEditable(true);
    restCompleter = new CustomCompletter(this);
    restCompleter->setWidget(this);
    this->setCompleter(nullptr);
    isReset=false;

    actionEdt = new QAction(QIcon(":images/key.png"),tr("Редактировать"),this);

    connect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(indexChanged(int)));
    connect(restCompleter,SIGNAL(currentDataChanged(colVal)),this,SLOT(setCurrentData(colVal)));
}

colVal RestComboBox::getCurrentData()
{
    return currentData;
}

RestComboBox::~RestComboBox()
{
    //qDebug()<<"delete RestComboBox";
}

void RestComboBox::setIndex(const QModelIndex &index)
{
    const RestTableModel *restModel = qobject_cast<const RestTableModel *>(index.model());
    if (restModel){
        dbModelIndex=index;
        if (!restModel->columnInfo(index.column()).relnam.isEmpty()){
            RestRelModel *model = RelModels::instance()->getModel(restModel->columnInfo(index.column()).relnam);
            if (this->model()!=model){
                this->setModel(model);
            }
            colVal c;
            c.disp=restModel->data(index,Qt::DisplayRole).toString();
            c.val=restModel->data(index,Qt::EditRole);
            setCurrentData(c);
        }
    }
}

void RestComboBox::setModel(QAbstractItemModel *model)
{
    RestRelModel *relModel = qobject_cast<RestRelModel *>(model);
    if (relModel){

        QComboBox::setModel(relModel);
        setModelColumn(1);

        connect(relModel,SIGNAL(refreshFinished(QString)),this,SLOT(updData()));
        connect(relModel,SIGNAL(modelAboutToBeReset()),this,SLOT(mAboutReset()));
        connect(relModel,SIGNAL(modelReset()),this,SLOT(mReset()));

        RestRelModel *likeModel = new RestRelModel(relModel->getName(),this);
        restCompleter->setModel(likeModel);
        restCompleter->setCompletionColumn(1);

        return;
    }
    return QComboBox::setModel(model);
}


void RestComboBox::indexChanged(int n)
{
    if (n>=0 && !isReset){
        colVal newVal;
        newVal.val=this->model()->data(this->model()->index(n,0),Qt::EditRole);
        newVal.disp=this->model()->data(this->model()->index(n,1),Qt::EditRole).toString();
        currentData=newVal;
    }
}

void RestComboBox::edtRel()
{
    emit sigActionEdtRel(dbModelIndex);
}

void RestComboBox::updData()
{
    this->blockSignals(true);
    this->setCurrentData(currentData);
    this->blockSignals(false);
}

void RestComboBox::mAboutReset()
{
    isReset=true;
    saveData=currentData;
}

void RestComboBox::mReset()
{
    isReset=false;
    setCurrentData(saveData);
}

void RestComboBox::setCurrentData(colVal data)
{
    currentData=data;
    bool ok=false;
    if (this->model() && !currentData.val.isNull()){
        for (int i=0; i<this->model()->rowCount(); i++){
            if (this->model()->data(this->model()->index(i,0),Qt::EditRole)==currentData.val){
                this->setCurrentIndex(i);
                ok=true;
                break;
            }
        }
    }
    if (!ok){
        this->setCurrentIndex(-1);
        if (this->isEditable()){
            this->lineEdit()->setText(data.disp);
        }
    }
}

CustomCompletter::CustomCompletter(QObject *parent) : QCompleter(parent)
{
    setCompletionMode(QCompleter::PopupCompletion);
    setCaseSensitivity(Qt::CaseInsensitive);
    setWrapAround(false);
    connect(this,SIGNAL(activated(QModelIndex)),this,SLOT(setCurrentKey(QModelIndex)));
}

CustomCompletter::~CustomCompletter()
{
    //qDebug()<<"delete completer";
}

bool CustomCompletter::eventFilter(QObject *o, QEvent *e)
{
    if (e->type()==QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if ((keyEvent->text()=="\r" || keyEvent->key()==Qt::Key_Tab) && this->popup()->isVisible()) {
            if (this->popup()->currentIndex().isValid()){
                emit activated(this->popup()->model()->index(this->popup()->currentIndex().row(),1));
            } else if (this->popup()->model()->rowCount()){
                emit activated(this->popup()->model()->index(0,1));
            }
            return true;
        }
    }
    return QCompleter::eventFilter(o,e);
}

void CustomCompletter::setModel(QAbstractItemModel *c)
{
    RestRelModel *mod = qobject_cast<RestRelModel *>(c);
    if (mod){
        connect(mod,SIGNAL(refreshFinished(QString)),this,SLOT(actFinished(QString)));
    }
    return QCompleter::setModel(c);
}

void CustomCompletter::setWidget(QWidget *widget)
{
    RestComboBox *combo = qobject_cast<RestComboBox *>(widget);
    if (combo){
        connect(combo->lineEdit(),SIGNAL(textEdited(QString)),this,SLOT(actComp(QString)));
    }
    return QCompleter::setWidget(widget);
}

void CustomCompletter::actComp(QString s)
{
    RestRelModel *mod = qobject_cast<RestRelModel *>(this->model());
    if (mod){
        if (!s.isEmpty()){
            mod->refreshByPattern(s);
        } else {
            mod->clear();
        }
    }
}

void CustomCompletter::setCurrentKey(QModelIndex index)
{
    if (index.isValid()){
        colVal d;
        d.disp=this->popup()->model()->data(this->popup()->model()->index(index.row(),1),Qt::EditRole).toString();
        d.val=this->popup()->model()->data(this->popup()->model()->index(index.row(),0),Qt::EditRole);
        emit currentDataChanged(d);
    }
}

void CustomCompletter::actFinished(QString s)
{
    setCompletionPrefix(s);
    if (s.size()){
        complete();
    } else {
        this->popup()->close();
    }
}
