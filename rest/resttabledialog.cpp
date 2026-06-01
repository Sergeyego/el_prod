#include "resttabledialog.h"
#include "ui_resttabledialog.h"

RestTableDialog::RestTableDialog(QString tname, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RestTableDialog)
{
    ui->setupUi(this);
    model = new RestTableModel(tname,this);
    ui->tableView->setModel(model);
    loadSettings();
    connect(ui->pushButtonOk,SIGNAL(clicked(bool)),this,SLOT(accept()));
}

RestTableDialog::~RestTableDialog()
{
    saveSettings();
    delete ui;
}

void RestTableDialog::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->restoreGeometry(settings.value("edttable_geometry_"+model->tableInfoName()).toByteArray());
}

void RestTableDialog::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("edttable_geometry_"+model->tableInfoName(), this->saveGeometry());
}

QVariantList RestTableDialog::currentPk()
{
    QVariantList pk;
    int r=ui->tableView->currentIndex().row();
    if (r>=0){
        for (int i=0; i<model->columnCount(); i++){
            if (model->columnInfo(i).is_pk){
                pk.push_back(model->data(model->index(r,i),Qt::EditRole));
            }
        }
    }
    return pk;
}
