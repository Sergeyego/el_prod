#ifndef FORMPACK_H
#define FORMPACK_H

#include <QWidget>
#include "rest/resttablemodel.h"
#include "rest/restmapper.h"
#include "invoicemanager.h"
#include "dialogpackload/dialogpackload.h"

namespace Ui {
class FormPack;
}

class FormPack : public QWidget
{
    Q_OBJECT

public:
    explicit FormPack(QWidget *parent = nullptr);
    ~FormPack();

private:
    Ui::FormPack *ui;
    void loadSettings();
    void saveSettings();
    RestTableModel *modelNakl;
    RestTableModel *modelPack;
    RestTableModel *modelBreak;
    RestMapper *mapper;
    InvoiceManager *invManager;

private slots:
    void setDefaultValue();
    void upd();
    void updData(int ind);
    void calcSum();
    void nakl();
    void naklPer();
    void loadPack();
};

#endif // FORMPACK_H
