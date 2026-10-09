#include "relmodels.h"

RelModels::RelModels(QObject *parent) : QObject(parent)
{

}

RelModels *RelModels::instance()
{
    static RelModels _instance;
    return &_instance;
}

RestRelModel *RelModels::getModel(const QString &name)
{
    auto it = map.constFind(name);
    if (it != map.constEnd()) {
        return it.value();
    }
    auto *model = new RestRelModel(name, this);
    model->refresh();
    map.insert(name, model);
    return model;
}

RelModels::~RelModels()
{
    //qDebug()<<"delete rels";
}

void RelModels::updateRels(const QVector<RestTableModel *> &models)
{
    QSet<QString> relSet;
    for (RestTableModel *model : models){
        if (!model) continue;
        for (int i=0; i<model->columnCount(); i++){
            QString rel = model->columnInfo(i).relnam;
            if (!rel.isEmpty()){
                relSet.insert(rel);
            }
        }
    }
    for (const QString &rel : relSet){
        auto it = map.constFind(rel);
        if (it != map.constEnd()) {
            it.value()->refresh();
        }
    }
}

void RelModels::updateAllRels()
{
    for (RestRelModel *model : map.values()){
        model->refresh();
    }
}
