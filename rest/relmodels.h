#ifndef RELMODELS_H
#define RELMODELS_H

#include <memory>
#include <QObject>
#include <QApplication>
#include "rest/restrelmodel.h"
#include "rest/resttablemodel.h"

class RestTableModel;

class RelModels : public QObject
{
    Q_OBJECT
protected:
    explicit RelModels(QObject *parent = nullptr);

public:
    static RelModels *instance();
    RestRelModel* getModel(QString name);
    ~RelModels();
    void updateRels(QVector<RestTableModel*> models);
public slots:
    void updateAllRels();

private:
    static std::unique_ptr<RelModels> relModels_instance;
    QMap <QString, RestRelModel*> map;

};

#endif // RELMODELS_H
