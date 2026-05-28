#ifndef RESTTABLEDIALOG_H
#define RESTTABLEDIALOG_H

#include <QDialog>
#include <QSettings>
#include "rest/resttablemodel.h"

namespace Ui {
class RestTableDialog;
}

class RestTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RestTableDialog(QString tname, QWidget *parent = nullptr);
    ~RestTableDialog();
    RestTableModel *model;

private:
    Ui::RestTableDialog *ui;
    void loadSettings();
    void saveSettings();
};

#endif // RESTTABLEDIALOG_H
