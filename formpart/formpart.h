#ifndef FORMPART_H
#define FORMPART_H

#include <QWidget>
#include <QGroupBox>
#include <QScrollBar>
#include <QSettings>
#include "rest/resttablemodel.h"
#include "rest/restmapper.h"

namespace Ui {
class FormPart;
}

class FormPart : public QWidget
{
    Q_OBJECT

public:
    explicit FormPart(QWidget *parent = nullptr);
    ~FormPart();

private:
    Ui::FormPart *ui;
    RestTableModel *modelPart;
    RestTableModel *modelGlass;
    RestTableModel *modelZam;
    RestTableModel *modelZamBreak;
    RestTableModel *modelRab;
    RestTableModel *modelChem;
    RestTableModel *modelMech;
    RestMapper *mapper;

    void loadSettings();
    void saveSettings();

private slots:
    void setDefaultValue();
    void updPart();
    void refreshCont(int ind);
    void setCurrentChemDev();
    void loadChem();
    void insertChemSamp();
    //void refreshGlassData(QModelIndex index);
    void insertMark();
    void insertProvol();
    void insertPack();
};

#endif // FORMPART_H
