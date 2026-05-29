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

void RelModels::updateResls(QVector<RestTableModel *> models)
{
    QSet<QString> relSet;
    for (RestTableModel *model : models){
        for (int i=0; i<model->columnCount(); i++){
            QString rel = model->columnInfo(i).relnam;
            if (!rel.isEmpty()){
                relSet.insert(rel);
            }
        }
    }
    for (const QString &rel : relSet){
        getModel(rel)->refresh();
    }
}

void RelModels::updateAllRels()
{
    for (RestRelModel *model : map.values()){
        model->refresh();
    }
}
