#ifndef RESTTABLEMODEL_H
#define RESTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QColor>
#include <QJsonArray>
#include "rest/httpsyncmanager.h"
#include "rest/restrelmodel.h"

struct colVal {
    QString disp;
    QVariant val;
    bool operator==(const colVal& rh) const {
        return (this->disp==rh.disp) && (this->val==rh.val);
    }
};

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
    Qt::ItemFlags flags;
    QVariant defaultVal;
};

class DataEditor : public QObject
{
    Q_OBJECT
public:
    DataEditor(QVector<QVector<cellData>> *data, QObject *parent=nullptr);
    bool add(int p, QVector<cellData> &row);
    bool edt(int row, int col, cellData val);
    void submit();
    void revert();
    bool isAdd();
    bool isEdt();
    int currentPos();
    QVector<cellData> oldRow();
    QVector<cellData> newRow();
private:
    QVector<QVector<cellData>> *mData;
    QVector<cellData> saveRow;
    bool addFlag;
    bool edtFlag;
    int pos;
};

class RestTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    RestTableModel(QString name, QObject *parent);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent=QModelIndex()) const;
    virtual bool insertRow(int row, const QModelIndex &parent=QModelIndex());
    virtual bool removeRow(int row, const QModelIndex &parent=QModelIndex());
    void setFilter(QString f);
    void setPath(QString p);
    void setInsertable(bool b);
    void setDefaultValue(int column, QVariant value);
    void setColumnFlags(int column, Qt::ItemFlags flags);
    QVariant defaultValue(int column) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role=Qt::EditRole);
    QMetaType::Type columnType(int col) const;
    QString tableName() const;
    colInfo columnInfo(int col) const;
    bool isColumnRel(int col) const;
    bool isAdd() const;
    bool isEdt() const;
    bool isEmpty() const;
    bool isInsertable() const;
    int currentEdtRow() const;
public slots:
    virtual void select();
    virtual void revert();
    virtual bool submitRow();
private:
    DataEditor *editor;
    QVector<QVector<cellData>> modelData;
    QVector<colInfo> colData;
    QString _path;
    QString _rname;
    QString _tablename;
    QString _filter;
    bool block;
    bool insertable;
    void loadInfo();
    QMetaType::Type getMetaType(const QString &udt_name) const;
    QVariant loadEdtVal(const QJsonValue &val, const QString &udt_name) const;
    QString formatVal(const QVariant &val, int column);

signals:
    void sigUpd();
    void sigRefresh();
    void sigAboutToBeInsert();
};

#endif // RESTTABLEMODEL_H
