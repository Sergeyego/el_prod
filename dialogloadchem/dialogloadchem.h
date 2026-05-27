#ifndef DIALOGLOADCHEM_H
#define DIALOGLOADCHEM_H

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QFileSystemModel>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QXmlStreamReader>
#include <QFile>
#include <math.h>
#include <QTimer>

namespace Ui {
class DialogLoadChem;
}

class DialogLoadChem : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLoadChem(QWidget *parent = 0);
    ~DialogLoadChem();
    double chemVal(QString chem);
    QMap <QString, double> chemVals();

private:
    Ui::DialogLoadChem *ui;
    QFileSystemModel *fileModel;
    void loadXmlIds(QXmlStreamReader &reader);
    void loadXmlStat(QXmlStreamReader &reader);
    QString loadXmlText(QXmlStreamReader &reader, QString tag);
    QMap <QString, double> chemMap;

private slots:
    void setCurrentDir();
    void setCurrentDir(QString path);
    void updDir();
    void parceXml(QModelIndex index);
};

#endif // DIALOGLOADCHEM_H
