#ifndef TABMANAGER_H
#define TABMANAGER_H

#include <QObject>
#include <QTabWidget>
#include <QSettings>
#include <QApplication>

class TabManager : public QObject
{
    Q_OBJECT
public:
    explicit TabManager(QTabWidget *tab, QObject *parent = nullptr);
    bool exist(QObject *a);
    void actAction(QAction *a);
    void addSubWindow(QWidget *w, QObject *a);
    bool setActiveSubWindow(QString t);

public slots:
    void closeTab(int index);
    void loadSettings();
    void saveSettings();

private:
    QTabWidget *tabWidget;
    QMap <QString,QAction*> actions;
};

#endif // TABMANAGER_H
