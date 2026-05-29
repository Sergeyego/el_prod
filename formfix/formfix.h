#ifndef FORMFIX_H
#define FORMFIX_H

#include <QWidget>
#include "rest/resttablemodel.h"
#include "rest/restrotablemodel.h"

namespace Ui {
class FormFix;
}

class FormFix : public QWidget
{
    Q_OBJECT

public:
    explicit FormFix(QWidget *parent = nullptr);
    ~FormFix();

private:
    Ui::FormFix *ui;
    RestTableModel *modelFix;
    RestRoTableModel *modelDat;

private slots:
    void updFix();
    void updFixData(int index);
    void fixNewOst();
};

#endif // FORMFIX_H
