#ifndef RESTITEMDELEGATE_H
#define RESTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QTimeEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QKeyEvent>
#include "rest/resttablemodel.h"
#include "rest/restcombobox.h"
#include "rest/restdateedit.h"
#include "rest/resttabledialog.h"
#include "rest/relmodels.h"

class RestItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RestItemDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    bool eventFilter(QObject *object, QEvent *event) override;

signals:
    void createEdt(QModelIndex index) const;

private slots:
    void edtRels(QModelIndex index);

private:
    // Помощники
    static void installIntValidator(QLineEdit *line, int min = INT_MIN, int max = INT_MAX);
    static void installDoubleValidator(QLineEdit *line, int decimals);

};

#endif // RESTITEMDELEGATE_H