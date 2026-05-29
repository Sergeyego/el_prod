#include "tabmanager.h"

TabManager::TabManager(QTabWidget *tab, QObject *parent) : QObject{parent}, tabWidget(tab)
{
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    connect(tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int)));
}

bool TabManager::exist(QObject *a)
{
    bool b=false;
    QAction *action = qobject_cast<QAction *>(a);
    if (action){
        b=setActiveSubWindow(action->text());
    }
    return b;
}

void TabManager::actAction(QAction *a)
{
    actions.insert(a->text(),a);
}

void TabManager::addSubWindow(QWidget *w, QObject *a)
{
    w->setAttribute(Qt::WA_DeleteOnClose);
    QAction *action = qobject_cast<QAction *>(a);
    if (action){
        w->setWindowTitle(action->text());
    }
    tabWidget->addTab(w,w->windowTitle());
    tabWidget->setCurrentWidget(w);
}

bool TabManager::setActiveSubWindow(QString t)
{
    bool b=false;
    for (int i=0; i<tabWidget->count(); i++){
        if (tabWidget->tabText(i)==t){
            tabWidget->setCurrentIndex(i);
            b=true;
            break;
        }
    }
    return b;
}

void TabManager::closeTab(int index)
{
    tabWidget->widget(index)->close();
}

void TabManager::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    QString opentab=settings.value("main_opentab").toString();
    QString current=settings.value("main_currenttab").toString();
    if (!opentab.isEmpty()){
        QStringList l=opentab.split("|");
        foreach (QString a, l) {
            if (actions.contains(a)){
                actions.value(a)->trigger();
            }
        }
    }
    setActiveSubWindow(current);
}

void TabManager::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    QString opentab, currenttab;
    for (int i=0; i<tabWidget->count(); i++){
        if (!opentab.isEmpty()){
            opentab+="|";
        }
        opentab+=tabWidget->tabText(i);
    }
    currenttab=tabWidget->tabText(tabWidget->currentIndex());
    settings.setValue("main_opentab", opentab);
    settings.setValue("main_currenttab",currenttab);
}
