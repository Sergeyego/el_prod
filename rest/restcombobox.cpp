#include "restcombobox.h"

// ============================================================
// CustomBaseCompleter
// ============================================================
CustomBaseCompleter::CustomBaseCompleter(QObject *parent) : QCompleter(parent)
{
    setCompletionMode(QCompleter::PopupCompletion);
    setCaseSensitivity(Qt::CaseInsensitive);
    setWrapAround(false);
}

bool CustomBaseCompleter::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);

        const bool isCommitKey =
            keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter  ||
            keyEvent->key() == Qt::Key_Tab;

        if (isCommitKey && popup() && popup()->isVisible()) {
            QAbstractItemModel *m = popup()->model();
            if (m) {
                QModelIndex current = popup()->currentIndex();
                if (current.isValid()) {
                    emit activated(m->index(current.row(), 1));
                } else if (m->rowCount() > 0) {
                    emit activated(m->index(0, 1));
                }
            }
            return true;
        }
    }
    return QCompleter::eventFilter(o, e);
}

// ============================================================
// CustomOnlineCompleter
// ============================================================
CustomOnlineCompleter::CustomOnlineCompleter(QObject *parent) : CustomBaseCompleter(parent)
{
    connect(this,
            static_cast<void(QCompleter::*)(const QModelIndex &)>(&QCompleter::activated),
            this, &CustomOnlineCompleter::setCurrentKey);
}

CustomOnlineCompleter::~CustomOnlineCompleter() = default;

void CustomOnlineCompleter::setModel(QAbstractItemModel *c)
{
    if (auto *mod = qobject_cast<RestRelModel *>(c)) {
        connect(mod, &RestRelModel::refreshFinished,
                this, &CustomOnlineCompleter::actFinished,
                Qt::UniqueConnection);
    }
    CustomBaseCompleter::setModel(c);
}

void CustomOnlineCompleter::setWidget(QWidget *widget)
{
    if (auto *combo = qobject_cast<RestComboBox *>(widget)) {
        if (combo->lineEdit()) {
            connect(combo->lineEdit(), &QLineEdit::textEdited,
                    this, &CustomOnlineCompleter::actComp,
                    Qt::UniqueConnection);
        }
    }
    CustomBaseCompleter::setWidget(widget);
}

void CustomOnlineCompleter::actComp(const QString &s)
{
    auto *mod = qobject_cast<RestRelModel *>(this->model());
    if (!mod) return;

    if (!s.isEmpty()) {
        mod->refreshByPattern(s);
    } else {
        mod->clear();
    }
}

void CustomOnlineCompleter::setCurrentKey(const QModelIndex &index)
{
    if (!index.isValid()) return;

    QAbstractItemModel *m = popup() ? popup()->model() : nullptr;
    if (!m) m = this->model();
    if (!m) return;

    if (auto *rel = qobject_cast<RestRelModel *>(m)) {
        colVal d = rel->getModelData(index.row());
        if (!d.val.isNull() || !d.disp.isEmpty()) {
            emit currentDataChanged(d);
        }
        return;
    }

    colVal d;
    d.disp = m->data(m->index(index.row(), 1), Qt::EditRole).toString();
    d.val  = m->data(m->index(index.row(), 0), Qt::EditRole);
    emit currentDataChanged(d);
}

void CustomOnlineCompleter::actFinished(const QString &s)
{
    setCompletionPrefix(s);
    if (!s.isEmpty()) {
        complete();
    } else if (popup()) {
        popup()->close();
    }
}

// ============================================================
// CustomOfflineCompleter
// ============================================================
CustomOfflineCompleter::CustomOfflineCompleter(QObject *parent) : CustomBaseCompleter(parent) {}
CustomOfflineCompleter::~CustomOfflineCompleter() = default;

// ============================================================
// RestComboBox
// ============================================================
RestComboBox::RestComboBox(QWidget *parent) : QComboBox(parent)
{
    setEditable(true);
    actionEdt = new QAction(QIcon(":images/key.png"), tr("Редактировать"), this);
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RestComboBox::indexChanged);
    connect(this, &QComboBox::currentTextChanged,
            this, &RestComboBox::textChanged);
}

RestComboBox::~RestComboBox()
{
    cleanupCompleter();
}

colVal RestComboBox::getCurrentData() const
{
    return currentData;
}

void RestComboBox::cleanupCompleter()
{
    if (isEditable()){
        setCompleter(nullptr);
    }

    if (m_onlineCompleter) {
        m_onlineCompleter->deleteLater();
        m_onlineCompleter = nullptr;
    }
    if (m_offlineCompleter) {
        m_offlineCompleter->deleteLater();
        m_offlineCompleter = nullptr;
    }
    if (m_likeModel) {
        m_likeModel->deleteLater();
        m_likeModel = nullptr;
    }
}

void RestComboBox::ensureEditAction()
{
    if (m_editActionAdded) return;

    if (lineEdit()) {
        lineEdit()->addAction(actionEdt, QLineEdit::TrailingPosition);
        connect(actionEdt, &QAction::triggered, this, &RestComboBox::edtRel);
        m_editActionAdded = true;
    }
}

void RestComboBox::setIndex(const QModelIndex &index)
{
    if (!index.isValid()) return;

    const auto *restModel = qobject_cast<const RestTableModel *>(index.model());
    if (!restModel) return;

    dbModelIndex = index;

    const QString relnam = restModel->columnInfo(index.column()).relnam;
    if (relnam.isEmpty()) return;

    RestRelModel *model = RelModels::instance()->getModel(relnam);
    if (this->model() != model) {
        this->setModel(model);
    }

    colVal c;
    c.disp = restModel->data(index, Qt::DisplayRole).toString();
    c.val  = restModel->data(index, Qt::EditRole);
    setCurrentData(c);
}

void RestComboBox::setModel(QAbstractItemModel *model)
{
    auto *relModel = qobject_cast<RestRelModel *>(model);
    if (!relModel) {
        return QComboBox::setModel(model);
    }

    cleanupCompleter();

    QComboBox::setModel(relModel);
    setModelColumn(1);

    connect(relModel, &RestRelModel::refreshFinished,
            this, &RestComboBox::updData,
            Qt::UniqueConnection);
    connect(relModel, &QAbstractItemModel::modelAboutToBeReset,
            this, &RestComboBox::mAboutReset,
            Qt::UniqueConnection);
    connect(relModel, &QAbstractItemModel::modelReset,
            this, &RestComboBox::mReset,
            Qt::UniqueConnection);

    if (isEditable()){
        if (relModel->isLimited()) {
            m_onlineCompleter = new CustomOnlineCompleter(this);
            m_onlineCompleter->setWidget(this);

            m_likeModel = new RestRelModel(relModel->getName(), m_onlineCompleter);
            m_onlineCompleter->setModel(m_likeModel);
            m_onlineCompleter->setCompletionColumn(1);

            connect(m_onlineCompleter, &CustomOnlineCompleter::currentDataChanged,
                    this, &RestComboBox::setCurrentData,
                    Qt::UniqueConnection);

            QComboBox::setCompleter(nullptr);
        } else {
            m_offlineCompleter = new CustomOfflineCompleter(this);
            m_offlineCompleter->setModel(relModel);
            m_offlineCompleter->setCompletionColumn(1);
            QComboBox::setCompleter(m_offlineCompleter);
        }

        if (relModel->isEditable()) {
            ensureEditAction();
        }
    }
}

void RestComboBox::indexChanged(int n)
{
    if (n < 0 || isReset) return;

    colVal newVal;
    newVal.val  = model()->data(model()->index(n, 0), Qt::EditRole);
    newVal.disp = model()->data(model()->index(n, 1), Qt::EditRole).toString();
    currentData = newVal;
}

void RestComboBox::textChanged(const QString &t)
{
    if (t.isEmpty()){
        colVal newVal;
        newVal.val = QVariant(); // Используем null, а не -1
        currentData = newVal;
        setCurrentIndex(-1);
    }
}

void RestComboBox::edtRel()
{
    emit sigActionEdtRel(dbModelIndex);
}

void RestComboBox::updData()
{
    blockSignals(true);
    setCurrentData(currentData);
    blockSignals(false);
}

void RestComboBox::mAboutReset()
{
    blockSignals(true);
    isReset = true;
    saveData = currentData;
}

void RestComboBox::mReset()
{
    isReset = false;
    setCurrentData(saveData);
    blockSignals(false);
}

void RestComboBox::setCurrentData(colVal data)
{
    // Блокируем сигналы, чтобы setText("") не вызвал textChanged
    // и не затер только что установленный currentData
    const QSignalBlocker blocker(this);

    currentData = data;

    // Ищем в модели только если ключ валиден и не пуст
    if (data.val.isValid() && !data.val.isNull()) {
        if (auto *rel = qobject_cast<RestRelModel *>(model())) {
            int row = rel->findRowByKey(data.val);
            if (row >= 0) {
                setCurrentIndex(row);
                return;
            }
        }
    }

    setCurrentIndex(-1);
    if (isEditable() && lineEdit()) {
        lineEdit()->setText(data.disp);
    }
}