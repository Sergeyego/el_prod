#ifndef RESTRELMODEL_H
#define RESTRELMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QUrlQuery>
#include "rest/restconnection.h"

class RestRelModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RestRelModel(QString name, QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QString getName() const;
public slots:
    void refresh();
    void refreshByPattern(QString pattern);
    void clear();

signals:
    void refreshFinished(QString pattern);

private:
    QString _name;
    QNetworkAccessManager *manager;
    QVector<QVector<QVariant>> _data;

private slots:
    void onResult(QNetworkReply *reply);
};

#endif // RESTRELMODEL_H
