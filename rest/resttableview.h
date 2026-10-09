#ifndef RESTTABLEVIEW_H
#define RESTTABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QHeaderView>
#include <QMenu>
#include <QSettings>
#include <QDir>
#include <QDesktopServices>
#include <QFileDialog>
#include "rest/resttablemodel.h"
#include "rest/restitemdelegate.h"
#include "rest/restrotablemodel.h"
#include "rest/tempfilemanager.h"

struct xlsxCol {
    QString key;
    QString header;
    int id_type;
    int dec;
    int width;
};

class RestTableView : public QTableView
{
    Q_OBJECT
public:
    RestTableView(QWidget *parent = nullptr);
    void setModel(QAbstractItemModel *model);
    void setDefaultDecimal(int dec);
    QString getTitle();

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;

private:
    QAction *updAct;
    QAction *removeAct;
    QAction *xlsxAct;
    QAction *excelAct;
    bool menuEnabled;
    bool writeOk;
    int getSpace(int column);
    RestTableModel *restModel;
    RestRoTableModel *restRoModel;
    int _dec;
    QJsonObject getJsonData();

    int m_defaultMinHeaderWidth;
    int m_defaultMaxHeaderWidth;
    QHeaderView::ResizeMode m_defaultHeaderResizeMode;
    QAbstractItemView::SelectionMode m_defaultSelectionMode;

    bool m_isInitialStateSaved = false;

public slots:
    void resizeToContents();
    void setMenuEnabled(bool value);
    void saveXlsx();
    void viewExcel();

private slots:
    void upd();
    void remove();
    void submit(QModelIndex ind, QModelIndex oldInd);
    void saveXlsxFinished();
    void viewExcelFinished();

};

#endif // RESTTABLEVIEW_H
