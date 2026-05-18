#ifndef FORMREPORT_H
#define FORMREPORT_H

#include <QWidget>
#include "rest/restrotablemodel.h"

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
    RestRoTableModel *model;
    //ProgressExecutor *executor;

private slots:
    void upd();
    void updFinished();
    void save();

};

#endif // FORMREPORT_H
