#ifndef RELMODELS_H
#define RELMODELS_H

#include <QObject>
#include "rest/resttablemodel.h"
#include "rest/restrelmodel.h"

class RestTableModel;

class RelModels : public QObject
{
    Q_OBJECT
protected:
    explicit RelModels(QObject *parent = nullptr);

public:
    static RelModels *instance();
    RestRelModel* getModel(const QString &name);
    ~RelModels();
    void updateRels(const QVector<RestTableModel*> &models);
public slots:
    void updateAllRels();

private:
    QMap <QString, RestRelModel*> map;

};

#endif // RELMODELS_H
