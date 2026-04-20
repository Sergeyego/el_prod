#ifndef RESTITEMDELEGATE_H
#define RESTITEMDELEGATE_H

#include <QItemDelegate>
#include <QObject>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QKeyEvent>
#include "rest/resttablemodel.h"

class RestItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    RestItemDelegate(QObject *parent = nullptr);
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    bool eventFilter(QObject *object, QEvent *event);

signals:
    void createEdt(const QModelIndex index) const;
};

#endif // RESTITEMDELEGATE_H
