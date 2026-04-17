#ifndef RESTTABLEMODEL_H
#define RESTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QColor>
#include <QJsonArray>
#include "rest/httpsyncmanager.h"

struct cellData {
    QString display;
    QVariant edit;
    QColor background;
    QString tooltip;
};

struct colInfo {
    QString nam;
    QString col;
    QString snam;
    QString udt_name;
    bool is_pk;
    bool editale;
    bool checkable;
    int dec;
    QString relnam;
};

class RestTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    RestTableModel(QString name, QObject *parent);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QMetaType::Type columnType(int col) const;
public slots:
    void select();
private:
    QVector<QVector<cellData>> modelData;
    QVector<colInfo> colData;
    QString _path;
    QString _rname;
    QString _tablename;
    void loadInfo();
    QMetaType::Type getMetaType(const QString &udt_name) const;
    QVariant loadEdtVal(const QJsonValue &val, const QString &udt_name) const;
};

#endif // RESTTABLEMODEL_H
