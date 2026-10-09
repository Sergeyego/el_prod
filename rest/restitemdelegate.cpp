#include "restitemdelegate.h"

RestItemDelegate::RestItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

// ============================================================
// createEditor
// ============================================================
QWidget *RestItemDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    const auto *restTableModel = qobject_cast<const RestTableModel *>(index.model());
    if (!restTableModel) {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    QWidget *editor = nullptr;

    if (restTableModel->isColumnRel(index.column())) {
        editor = new RestComboBox(parent);
    } else {
        switch (restTableModel->columnType(index.column())) {
        case QMetaType::Bool:
            editor = nullptr; // используется CheckStateRole
            break;

        case QMetaType::Int:
            // Если колонка отображается как чекбокс — редактор не нужен
            if (!restTableModel->columnInfo(index.column()).checkable) {
                editor = new QLineEdit(parent);
            }
            break;

        case QMetaType::LongLong:
        case QMetaType::Double:
        case QMetaType::QString:
            editor = new QLineEdit(parent);
            break;

        case QMetaType::QDate:
            editor = new RestDateEdit(parent);
            break;

        case QMetaType::QDateTime:
            editor = new RestDateTimeEdit(parent);
            break;

        case QMetaType::QTime: {
            auto *te = new QTimeEdit(parent);
            te->setDisplayFormat(QStringLiteral("HH:mm:ss"));
            editor = te;
            break;
        }

        default:
            editor = QStyledItemDelegate::createEditor(parent, option, index);
            break;
        }
    }

    if (editor) {
        editor->installEventFilter(const_cast<RestItemDelegate *>(this));
        emit createEdt(index);
    }
    return editor;
}

// ============================================================
// setEditorData
// ============================================================
void RestItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const auto *restTableModel = qobject_cast<const RestTableModel *>(index.model());
    if (!restTableModel) {
        return QStyledItemDelegate::setEditorData(editor, index);
    }

    // --- Rel-колонка ---
    if (restTableModel->isColumnRel(index.column())) {
        if (auto *combo = qobject_cast<RestComboBox *>(editor)) {
            // Всегда подключаем сигнал редактирования справочника.
            // Раньше это было под условием сравнения моделей — из-за чего кнопка
            // «Редактировать» иногда не работала. UniqueConnection защищает от дублей.
            connect(combo, &RestComboBox::sigActionEdtRel,
                    this, &RestItemDelegate::edtRels,
                    Qt::UniqueConnection);

            combo->setIndex(index);
            return;
        }
        // Fallback: line edit с текстом из DisplayRole
        if (auto *le = qobject_cast<QLineEdit *>(editor)) {
            le->setText(index.model()->data(index, Qt::DisplayRole).toString());
            return;
        }
        return QStyledItemDelegate::setEditorData(editor, index);
    }

    const QVariant dat = restTableModel->data(index, Qt::EditRole);

    switch (restTableModel->columnType(index.column())) {
    case QMetaType::Int:
    case QMetaType::LongLong: {
        if (auto *line = qobject_cast<QLineEdit *>(editor)) {
            if (!line->validator()) {
                installIntValidator(line);
            }
            if (dat.isNull()) line->clear();
            else              line->setText(dat.toString());
            return;
        }
        break;
    }

    case QMetaType::Double: {
        if (auto *line = qobject_cast<QLineEdit *>(editor)) {
            if (!line->validator()) {
                installDoubleValidator(line, restTableModel->columnInfo(index.column()).dec);
            }
            if (dat.isNull()) {
                line->clear();
            } else {
                line->setText(QString::number(dat.toDouble(), 'f',
                                              restTableModel->columnInfo(index.column()).dec));
            }
            return;
        }
        break;
    }

    case QMetaType::QDate: {
        if (auto *de = qobject_cast<RestDateEdit *>(editor)) {
            de->setDate(dat.isNull() ? de->minimumDate() : dat.toDate());
            return;
        }
        break;
    }

    case QMetaType::QDateTime: {
        if (auto *dte = qobject_cast<RestDateTimeEdit *>(editor)) {
            dte->setDateTime(dat.isNull() ? dte->minimumDateTime() : dat.toDateTime());
            return;
        }
        break;
    }

    case QMetaType::QTime: {
        if (auto *te = qobject_cast<QTimeEdit *>(editor)) {
            if (dat.isNull()) {
                te->setTime(QTime(0, 0, 0));
            } else {
                te->setTime(dat.toTime());
            }
            return;
        }
        break;
    }

    default:
        break;
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

// ============================================================
// setModelData
// ============================================================
void RestItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    if (!index.isValid()) return;

    auto *restTableModel = qobject_cast<RestTableModel *>(model);
    if (!restTableModel) {
        return QStyledItemDelegate::setModelData(editor, model, index);
    }

    const QMetaType::Type type = restTableModel->columnType(index.column());

    // --- Rel-колонка ---
    if (restTableModel->isColumnRel(index.column())) {
        if (auto *combo = qobject_cast<RestComboBox *>(editor)) {
            colVal data = combo->getCurrentData();
            if (combo->currentText().isEmpty()) {
                data.val  = restTableModel->nullValue(index.column());
                data.disp = QString();
            }
            // Одна запись — два setData: сначала ключ, потом текст.
            // RestTableModel::setData для rel не пересчитывает display автоматически,
            // поэтому disp задаём отдельно.
            restTableModel->setData(index, data.val,  Qt::EditRole);
            restTableModel->setData(index, data.disp, Qt::DisplayRole);
            return;
        }
        return;
    }

    // --- Line edit ---
    if (auto *le = qobject_cast<QLineEdit *>(editor)) {
        if (le->text().isEmpty()) {
            restTableModel->setData(index, restTableModel->nullValue(index.column()), Qt::EditRole);
            return;
        }

        switch (type) {
        case QMetaType::Int: {
            bool ok = false;
            const int v = le->text().toInt(&ok);
            if (ok) restTableModel->setData(index, v, Qt::EditRole);
            break;
        }
        case QMetaType::LongLong: {
            bool ok = false;
            const qlonglong v = le->text().toLongLong(&ok);
            if (ok) restTableModel->setData(index, v, Qt::EditRole);
            break;
        }
        case QMetaType::Double: {
            bool ok = false;
            const double v = le->text().toDouble(&ok);
            if (ok) restTableModel->setData(index, v, Qt::EditRole);
            break;
        }
        default:
            restTableModel->setData(index, le->text(), Qt::EditRole);
            break;
        }
        return;
    }

    // --- Date ---
    if (auto *de = qobject_cast<RestDateEdit *>(editor)) {
        if (de->date() == de->minimumDate()) {
            restTableModel->setData(index, restTableModel->nullValue(index.column()), Qt::EditRole);
        } else {
            restTableModel->setData(index, de->date(), Qt::EditRole);
        }
        return;
    }

    // --- DateTime ---
    if (auto *dte = qobject_cast<RestDateTimeEdit *>(editor)) {
        if (dte->dateTime() == dte->minimumDateTime()) {
            restTableModel->setData(index, restTableModel->nullValue(index.column()), Qt::EditRole);
        } else {
            restTableModel->setData(index, dte->dateTime(), Qt::EditRole);
        }
        return;
    }

    // --- Time ---
    if (auto *te = qobject_cast<QTimeEdit *>(editor)) {
        restTableModel->setData(index, te->time(), Qt::EditRole);
        return;
    }

    QStyledItemDelegate::setModelData(editor, model, index);
}

// ============================================================
// eventFilter
// ============================================================
bool RestItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        // Коммитим и закрываем редактор на Tab/Up/Down
        const bool isCommit =
            keyEvent->key() == Qt::Key_Tab ||
            keyEvent->key() == Qt::Key_Down ||
            keyEvent->key() == Qt::Key_Up;

        if (isCommit) {
            if (auto *editor = qobject_cast<QWidget *>(object)) {
                emit commitData(editor);
                emit closeEditor(editor);
            }
            // НЕ поглощаем — даём Qt обработать навигацию дальше
            return false;
        }

        // Замена ',' на '.' для QDoubleValidator
        if (auto *line = qobject_cast<QLineEdit *>(object)) {
            if (qobject_cast<const QDoubleValidator *>(line->validator())) {
                if (keyEvent->text() == QLatin1String(",")) {
                    line->insert(QLatin1String("."));
                    return true;
                }
            }
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}

// ============================================================
// edtRels — редактирование справочника через диалог
// ============================================================
void RestItemDelegate::edtRels(QModelIndex index)
{
    if (!index.isValid()) return;

    auto *restTableModel = qobject_cast<RestTableModel *>(
        const_cast<QAbstractItemModel *>(index.model()));
    if (!restTableModel || !restTableModel->isColumnRel(index.column())) {
        return;
    }

    const colInfo ci = restTableModel->columnInfo(index.column());

    // Открываем диалог редактирования справочника
    RestTableDialog d(ci.relnam);
    d.setWindowTitle(tr("Редактирование таблицы ") + ci.snam);
    d.model->select();

    const auto dialogResult = d.exec();

    // Обновляем справочник независимо от результата — данные могли измениться
    if (auto *rel = RelModels::instance()->getModel(ci.relnam)) {
        rel->refresh();
    }

    if (dialogResult != QDialog::Accepted) {
        return;
    }

    const QVariantList pk = d.currentPk();

    if (pk.isEmpty()) {
        return;
    }

    if (pk.size() > 1) {
        // Составной PK в rel-колонке — не поддерживается текущей логикой.
        // Не молчим, а показываем пользователю.
        QMessageBox::warning(
            QApplication::activeWindow(),
            tr("Составной ключ"),
            tr("Справочник «%1» имеет составной первичный ключ. "
               "Автоматический выбор значения невозможен, выберите вручную.")
                .arg(ci.snam));
        return;
    }

    const QVariant newValue = pk.first();

    if (auto *combo = qobject_cast<RestComboBox *>(sender())) {
        // Обновляем комбобокс — значение запишется в модель при commitData
        colVal c;
        c.val = newValue;
        // disp подтянется из модели комбобокса при setCurrentData
        combo->setCurrentData(c);
    } else {
        // Диалог вызван не из комбобокса — пишем напрямую в модель
        restTableModel->setData(index, newValue, Qt::EditRole);
        restTableModel->setData(index,
                                restTableModel->formatVal(newValue, index.column()),
                                Qt::DisplayRole);
    }
}

// ============================================================
// Помощники для валидаторов
// ============================================================
void RestItemDelegate::installIntValidator(QLineEdit *line, int min, int max)
{
    auto *v = new QIntValidator(min, max, line);
    v->setLocale(QLocale::English);
    line->setValidator(v);
}

void RestItemDelegate::installDoubleValidator(QLineEdit *line, int decimals)
{
    auto *v = new QDoubleValidator(line);
    v->setLocale(QLocale::English);
    v->setDecimals(decimals);
    v->setNotation(QDoubleValidator::StandardNotation);
    line->setValidator(v);
}