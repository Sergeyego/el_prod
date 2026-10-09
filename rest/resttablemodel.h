#ifndef RESTTABLEMODEL_H
#define RESTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QColor>
#include <QJsonArray>
#include <QQueue>
#include <QApplication>
#include <QMessageBox>
#include "rest/restrelmodel.h"
#include "rest/relmodels.h"
#include "rest/restfilter.h"
#include "rest/resttypes.h"

class DataEditor : public QObject
{
    Q_OBJECT
public:
    DataEditor(QVector<QVector<cellData>> *data, QObject *parent=nullptr);
    bool add(int p, const QVector<cellData> &row);
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
    ~RestTableModel();
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent=QModelIndex()) const;
    virtual bool insertRow(int row, const QModelIndex &parent=QModelIndex());
    virtual bool removeRow(int row, const QModelIndex &parent=QModelIndex());
    virtual bool refreshRow(int row);
    void setFilter(const RestFilter &f);
    void setPath(QString p);
    void setInsertable(bool b);
    void setDefaultValue(QString column, QVariant value);
    void setColumnFlags(QString column, Qt::ItemFlags flags);
    void setColumns(const QStringList &cols);
    static QVariant nullValue(const QString &udt_name);
    QVariant nullValue(int column) const;
    QVariant defaultValue(int column) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role=Qt::EditRole);
    QMetaType::Type columnType(int col) const;
    QString tableName() const;
    QString tableInfoName() const;
    colInfo columnInfo(int col) const;
    QString path() const;
    RestFilter filter() const;
    QVariant getModelData(int row, QString col) const;
    bool isColumnRel(int col) const;
    bool isAdd() const;
    bool isEdt() const;
    bool isEmpty() const;
    bool isInsertable() const;
    int currentEdtRow() const;
    int columnIndex(QString nam) const;
    static QMetaType::Type getMetaType(const QString &udt_name);
    static QVariant loadEdtVal(const QJsonValue &val, const QString &udt_name);
    static QString variantToDbString(const QVariant &val);
    static QJsonValue getJsonValue(const QVariant &val);
    QString formatVal(const QVariant &val, int column) const;

public slots:
    virtual void select();
    virtual void selectSync();
    void clear();
    virtual void revert();
    virtual bool submitRow();

private slots:
    void processNextRequest();
    void onResult();

private:
    QQueue<QUrl> queue;
    bool isProcessing;
    DataEditor *editor;
    QVector<QVector<cellData>> modelData;
    QMap<QString,colInfo> colMap;
    QString _path;
    QString _rname;
    QString _tablename;
    RestFilter _filter;
    QStringList _columns;
    bool block;
    bool insertable;
    void loadInfo();
    void setModelData(const QJsonArray &data);
    bool apiInsert();
    bool apiUpdate();
    bool apiDelete(int row);

    QVector<cellData> loadRow(const QJsonValue &val) const;
    QVector<cellData> defaultRow() const;

    QJsonObject getRowObject(const QVector<cellData> &row);

signals:
    void sigUpd();
    void sigRefresh();
    void sigAboutToBeInsert();
};

#endif // RESTTABLEMODEL_H
