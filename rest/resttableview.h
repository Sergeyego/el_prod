#ifndef RESTTABLEVIEW_H
#define RESTTABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QHeaderView>
#include "rest/resttablemodel.h"
#include "rest/restitemdelegate.h"

class RestTableView : public QTableView
{
public:
    RestTableView(QWidget *parent = nullptr);

public slots:
    void resizeToContents();
};

#endif // RESTTABLEVIEW_H
