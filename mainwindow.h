#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QDir>
#include "rest/resttablemodel.h"
#include "rest/restrelmodel.h"
#include "rest/relmodels.h"
#include "tabmanager.h"
#include "formpart/formpart.h"
#include "formreport/formreport.h"
#include "rest_olap/cubewidget.h"
#include "formpack/formpack.h"
#include "formperepack/formperepack.h"
#include "formself/formself.h"
#include "formfix/formfix.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    TabManager *tabManager;
    void actAction(QAction *a, void (MainWindow::*sl)());
    void loadSettings();
    void saveSettings();
    void loadAnalytics();

private slots:
    void newFormPart();
    void newFormPack();
    void newFormPerePack();
    void newFormSelf();
    void newFormReport();
    void newFormFix();
    void newAnalytics();
};
#endif // MAINWINDOW_H
