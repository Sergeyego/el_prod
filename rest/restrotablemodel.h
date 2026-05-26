#ifndef RESTROTABLEMODEL_H
#define RESTROTABLEMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include "rest/resttablemodel.h"

class RestRoTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RestRoTableModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    colInfo columnInfo(int col) const;
    QMetaType::Type columnType(int col) const;
    void setPath(QString p);
    void setModelData(const QJsonObject &data);
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role=Qt::EditRole);
    QString title() const;
    QString path() const;
    QVariant getModelData(int row, QString col) const;

public slots:
    void select();

private slots:
    void onResult(QNetworkReply *reply);

private:
    QVector<QVector<cellData>> modelData;
    QMap<QString,colInfo> colMap;
    QString _path;
    QString _title;
    QStringList _columns;
    QNetworkAccessManager *manager;

signals :
    void sigRefresh();
    void sigStartRefresh();
};

#endif // RESTROTABLEMODEL_H
