#ifndef RESTMAPPER_H
#define RESTMAPPER_H

#include <QWidget>
#include <QDataWidgetMapper>
#include <QTableView>
#include <QListView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include "rest/resttablemodel.h"
#include "rest/restitemdelegate.h"

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
    QVector <QWidget*> lockedWidgets;
    QVector <QWidget*> unlockedWidgets;
    QVector <QWidget*> lockedEmptyWidgets;
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
    QVariantList savedPk;
    bool m_addPending = false;
    void insertNewRow();

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
    void reqSelect();
    void restorePos();


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
