#ifndef RESTMAPPER_H
#define RESTMAPPER_H

#include <QWidget>
#include "resttablemodel.h"
#include <QDataWidgetMapper>
#include "restitemdelegate.h"
#include <QTableView>
#include <QListView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>

class RestMapper : public QWidget
{
    Q_OBJECT
    
public:
    explicit RestMapper(QAbstractItemView *v, QWidget *parent = nullptr);
    ~RestMapper();
    void addLock(QWidget *widget);
    void addUnLock(QWidget *widget);
    void addEmptyLock(QWidget *widget);
    bool isLock();
    void addMapping(QWidget *widget, QString section);
    int currentIndex();
    void setDefaultFocus(int n);
    void setItemDelegate(QAbstractItemDelegate *delegate);
    QVariant modelData(int row, QString column);
    
private:
    QVector <QWidget*> lock1;
    QVector <QWidget*> lock2;
    QVector <QWidget*> lockEmpty;
    QDataWidgetMapper *mapper;
    QAbstractItemView *viewer;
    QPushButton *cmdNew;
    QPushButton *cmdWrite;
    QPushButton *cmdEdt;
    QPushButton *cmdEsc;
    QPushButton *cmdDel;
    QHBoxLayout *mainLayout;
    bool isEdt;
    int defaultFocus;

public slots:
    void refresh();
    void slotNew();
    void slotEdt();
    void slotDel();
    void slotWrite();
    void slotEsc();
    void first();
    void last();
    void setCurrentViewRow(int row);

private slots:
    void checkEmpty();
    void lock(bool val);

signals:
    void currentIndexChanged(int index);
    void lockChanged(bool l);
    void sigWrite();
    void sigEdt();
};

#endif // RESTMAPPER_H
