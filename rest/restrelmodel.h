#ifndef RESTRELMODEL_H
#define RESTRELMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QQueue>
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
    bool isLimited();
    void setPath(QString p);

public slots:
    void refresh();
    void refreshByPattern(QString pattern);
    void clear();

signals:
    void refreshFinished(QString pattern);

private:
    QString _name;
    QQueue<QUrl> queue;
    bool isProcessing;
    QVector<QVector<QVariant>> _data;
    bool _is_limited;
    QString _path;

private slots:
    void processNextRequest();
    void onResult();
};

#endif // RESTRELMODEL_H
