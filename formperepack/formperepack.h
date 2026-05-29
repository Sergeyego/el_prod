#ifndef FORMPEREPACK_H
#define FORMPEREPACK_H

#include <QWidget>
#include <QSettings>
#include "rest/resttablemodel.h"
#include "rest/restmapper.h"
#include "invoicemanager.h"

namespace Ui {
class FormPerePack;
}

class FormPerePack : public QWidget
{
    Q_OBJECT

public:
    explicit FormPerePack(QWidget *parent = nullptr);
    ~FormPerePack();

private:
    Ui::FormPerePack *ui;
    RestMapper *mapper;
    RestTableModel *modelNakl;
    RestTableModel *modelPerePack;
    InvoiceManager *invManager;
    void loadSettings();
    void saveSettings();

private slots:
    void setDefaultValue();
    void refreshRels();
    void upd();
    void updData(int ind);
    void calcSum();
    void nakl();
};

#endif // FORMPEREPACK_H
