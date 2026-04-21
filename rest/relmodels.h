#ifndef RELMODELS_H
#define RELMODELS_H

#include <QObject>
#include "rest/restrelmodel.h"

class RelModels : public QObject
{
    Q_OBJECT
protected:
    explicit RelModels(QObject *parent = nullptr);

public:
    static RelModels *instance();
    RestRelModel* getModel(QString name);

private:
    static RelModels *relModels_instance;
    QMap <QString, RestRelModel*> map;

signals:
};

#endif // RELMODELS_H
