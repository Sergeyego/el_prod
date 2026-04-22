#include "relmodels.h"

RelModels* RelModels::relModels_instance=nullptr;

RelModels::RelModels(QObject *parent) : QObject(parent)
{
    connect(qApp,SIGNAL(aboutToQuit()),this,SLOT(deleteLater()));
}

RelModels *RelModels::instance()
{
    if (relModels_instance==nullptr){
        relModels_instance = new RelModels();
    }
    return relModels_instance;
}

RestRelModel *RelModels::getModel(QString name)
{
    if (!map.contains(name)){
        RestRelModel *model = new RestRelModel(name,this);
        model->refresh();
        map.insert(name, model);
    }
    return map.value(name, nullptr);
}

RelModels::~RelModels()
{
    //qDebug()<<"delete rels";
}
