#ifndef FORMREPORT_H
#define FORMREPORT_H

#include <QWidget>
#include "rest/restrotablemodel.h"
#include "progressreportdialog/progressreportdialog.h"

namespace Ui {
class FormReport;
}

class FormReport : public QWidget
{
    Q_OBJECT
public:
    explicit FormReport(QWidget *parent = nullptr);
    ~FormReport();

private:
    Ui::FormReport *ui;
    RestRoTableModel *model;
    ProgressReportDialog *progressDialog;

private slots:
    void updPath();
};

#endif // FORMREPORT_H
