#ifndef RESTCOMBOBOX_H
#define RESTCOMBOBOX_H

#include <QComboBox>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QCompleter>
#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include "rest/resttablemodel.h"
#include "rest/relmodels.h"

class CustomCompletter : public QCompleter
{
    Q_OBJECT
public:
    CustomCompletter(QObject *parent=nullptr);
    ~CustomCompletter();
    bool eventFilter(QObject *o, QEvent *e);
    void setModel(QAbstractItemModel *c);
    void setWidget(QWidget *widget);
private slots:
    void actComp(QString s);
    void setCurrentKey(QModelIndex index);
    void actFinished(QString s);
signals:
    void currentDataChanged(colVal d);
};

class RestComboBox : public QComboBox
{
    Q_OBJECT
public:
    RestComboBox(QWidget *parent=nullptr);
    colVal getCurrentData();
    ~RestComboBox();
    void setIndex(const QModelIndex &index);
    void setModel(QAbstractItemModel *model);
private:
    CustomCompletter *restCompleter;
    colVal currentData;
    colVal saveData;
    QAction *actionEdt;
    QModelIndex dbModelIndex;
    bool isReset;

signals:
    void sigActionEdtRel(const QModelIndex &index);

private slots:
    void indexChanged(int n);
    void edtRel();
    void updData();
    void mAboutReset();
    void mReset();

public slots:
    void setCurrentData(colVal data);
};

#endif // RESTCOMBOBOX_H
