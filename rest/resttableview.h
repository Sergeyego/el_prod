#ifndef RESTTABLEVIEW_H
#define RESTTABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QHeaderView>
#include <QMenu>
#include "rest/resttablemodel.h"
#include "rest/restitemdelegate.h"

class RestTableView : public QTableView
{
    Q_OBJECT
public:
    RestTableView(QWidget *parent = nullptr);
    void setModel(QAbstractItemModel *model);

protected:
    virtual void keyPressEvent (QKeyEvent *e);
    virtual void contextMenuEvent(QContextMenuEvent *event);

private:
    QAction *updAct;
    QAction *removeAct;
    bool menuEnabled;
    bool writeOk;

public slots:
    void resizeToContents();
    void setMenuEnabled(bool value);

private slots:
    void upd();
    void remove();
    void submit(QModelIndex ind, QModelIndex oldInd);
    void focusOutEvent(QFocusEvent *event);
};

#endif // RESTTABLEVIEW_H
