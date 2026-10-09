#ifndef RESTRELMODEL_H
#define RESTRELMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QQueue>
#include <QJsonArray>
#include <QUrlQuery>
#include <QApplication>
#include <QMessageBox>
#include "rest/restconnection.h"
#include "rest/resttypes.h"


class RestRelModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RestRelModel(QString name, QObject *parent = nullptr);
    ~RestRelModel();
    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QString getName() const;
    QString editor() const;
    bool isLimited() const;
    void setPath(QString p);
    bool isEditable() const;
    void setEditable(bool e);
    colVal getModelData(int index) const;

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
    QList<colVal> _data;
    bool _is_limited;
    QString _path;
    QString _editor;
    bool _editable;

private slots:
    void processNextRequest();
    void onResult();
};

#endif // RESTRELMODEL_H
