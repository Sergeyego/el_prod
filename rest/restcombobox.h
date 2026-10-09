#ifndef RESTCOMBOBOX_H
#define RESTCOMBOBOX_H

#include <QComboBox>
#include <QCompleter>
#include <QLineEdit>
#include <QAction>
#include <QKeyEvent>
#include <QAbstractItemView>
#include "rest/resttablemodel.h"
#include "rest/restrelmodel.h"
#include "rest/relmodels.h"
#include "rest/resttypes.h"

class RestComboBox;

// ============================================================
// Базовый комплитер: общий eventFilter для Enter/Tab
// ============================================================
class CustomBaseCompleter : public QCompleter
{
    Q_OBJECT
public:
    explicit CustomBaseCompleter(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
};

// ============================================================
// Онлайн-комплитер: ищет через refreshByPattern на сервере
// ============================================================
class CustomOnlineCompleter : public CustomBaseCompleter
{
    Q_OBJECT
public:
    explicit CustomOnlineCompleter(QObject *parent = nullptr);
    ~CustomOnlineCompleter() override;

    void setModel(QAbstractItemModel *c);
    void setWidget(QWidget *widget);

signals:
    void currentDataChanged(colVal data);

private slots:
    void actComp(const QString &s);
    void actFinished(const QString &s);
    void setCurrentKey(const QModelIndex &index);
};

// ============================================================
// Офлайн-комплитер: работает по уже загруженной модели
// ============================================================
class CustomOfflineCompleter : public CustomBaseCompleter
{
    Q_OBJECT
public:
    explicit CustomOfflineCompleter(QObject *parent = nullptr);
    ~CustomOfflineCompleter() override;
};

// ============================================================
// RestComboBox
// ============================================================
class RestComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit RestComboBox(QWidget *parent = nullptr);
    ~RestComboBox() override;

    void setIndex(const QModelIndex &index);
    void setModel(QAbstractItemModel *model) override;
    colVal getCurrentData() const;
    void setCurrentData(colVal data);

signals:
    void sigActionEdtRel(QModelIndex index);

private slots:
    void indexChanged(int n);
    void edtRel();
    void updData();
    void mAboutReset();
    void mReset();

private:
    void cleanupCompleter();
    void ensureEditAction();

    QModelIndex dbModelIndex;
    colVal currentData;
    colVal saveData;
    bool isReset = false;

    QAction *actionEdt = nullptr;

    // Храним указатели, чтобы корректно удалять при смене модели
    CustomOnlineCompleter  *m_onlineCompleter  = nullptr;
    CustomOfflineCompleter *m_offlineCompleter = nullptr;
    RestRelModel           *m_likeModel        = nullptr;
    bool m_editActionAdded = false;
};

#endif // RESTCOMBOBOX_H
