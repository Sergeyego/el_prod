#ifndef DIALOGPACKLOAD_H
#define DIALOGPACKLOAD_H

#include <QDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QTextFrame>
#include <QTextFrameFormat>
#include <QDate>
#include "rest/restconnection.h"
#include "rest/restrotablemodel.h"

namespace Ui {
class DialogPackLoad;
}

class DialogPackLoad : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPackLoad(QWidget *parent = nullptr);
    ~DialogPackLoad();

private:
    Ui::DialogPackLoad *ui;
    void setHtml(const QString &html);
    void updMaster(QDate date);
    RestRoTableModel *modelMaster;
    QString currentIdMaster();
    int currentIdNakl;

public slots:
    void setCurrentDate(QDate date);
    void setIdNakl(int id);

private slots:
    void loadDoc();
    void print();
    void loadData();
};

#endif // DIALOGPACKLOAD_H
