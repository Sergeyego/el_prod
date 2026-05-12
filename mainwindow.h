#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include "rest/httpsyncmanager.h"
#include "rest/resttablemodel.h"
#include "rest/restrelmodel.h"
#include "rest/relmodels.h"
#include "tabmanager.h"
#include"formpart/formpart.h"

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

private slots:
    void newFormPart();
};
#endif // MAINWINDOW_H
