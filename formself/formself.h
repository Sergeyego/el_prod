#ifndef FORMSELF_H
#define FORMSELF_H

#include <QWidget>
#include <QSettings>
#include "rest/resttablemodel.h"
#include "rest/restmapper.h"
#include "invoicemanager.h"

namespace Ui {
class FormSelf;
}

class FormSelf : public QWidget
{
    Q_OBJECT

public:
    explicit FormSelf(QWidget *parent = nullptr);
    ~FormSelf();

private:
    Ui::FormSelf *ui;
    void loadSettings();
    void saveSettings();
    RestTableModel *modelSelfNakl;
    RestTableModel *modelSelfData;
    RestMapper *mapper;
    InvoiceManager *invManager;

private slots:
    void setDefaultValue();
    void refreshRels();
    void upd();
    void updData(int ind);
    void calcSum();
    void nakl();
    void naklPer();
};

#endif // FORMSELF_H
