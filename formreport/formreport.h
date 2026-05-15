#ifndef FORMREPORT_H
#define FORMREPORT_H

#include <QWidget>
//#include "db/tablemodel.h"
//#include "progressexecutor.h"

namespace Ui {
class FormReport;
}

/*class ModelReport : public TableModel
{
    Q_OBJECT

public:
    explicit ModelReport(QWidget *parent = nullptr);
    QVariant data(const QModelIndex &index, int role) const;
};*/

class FormReport : public QWidget
{
    Q_OBJECT

public:
    explicit FormReport(QWidget *parent = nullptr);
    ~FormReport();

private:
    Ui::FormReport *ui;
    //ModelReport *model;
    //ProgressExecutor *executor;

private slots:
    void upd();
    void updFinished();
    void save();

};

#endif // FORMREPORT_H
