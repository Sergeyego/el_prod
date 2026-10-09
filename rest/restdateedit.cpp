#include "restdateedit.h"

// ============================================================
// CustomCalendarWidget
// ============================================================
CustomCalendarWidget::CustomCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    setFirstDayOfWeek(Qt::Monday);
}

void CustomCalendarWidget::showEvent(QShowEvent *event)
{
    emit shown();
    QCalendarWidget::showEvent(event);
}

// ============================================================
// RestDateEdit
// ============================================================
RestDateEdit::RestDateEdit(QWidget *parent) : QDateEdit(parent)
{
    setCalendarPopup(true);

    auto *cw = new CustomCalendarWidget(this);
    setCalendarWidget(cw);

    setDisplayFormat(QStringLiteral("dd.MM.yy"));
    setSpecialValueText(QStringLiteral("NULL"));

    connect(lineEdit(), &QLineEdit::textChanged,
            this, &RestDateEdit::txtChangeSlot);
    connect(cw, &CustomCalendarWidget::shown,
            this, &RestDateEdit::onCalendarShown);
}

void RestDateEdit::setDate(const QDate &date)
{
    QDateEdit::setDate(date.isNull() ? minimumDate() : date);
}

void RestDateEdit::clear()
{
    QDateEdit::setDate(minimumDate());
}

void RestDateEdit::txtChangeSlot(const QString &txt)
{
    if (txt.isEmpty()) {
        blockSignals(true);
        setDate(minimumDate());
        blockSignals(false);
    }
}

void RestDateEdit::onCalendarShown()
{
    if (date() == minimumDate()) {
        setDate(QDate::currentDate());
    }
}

// ============================================================
// RestDateTimeEdit
// ============================================================
RestDateTimeEdit::RestDateTimeEdit(QWidget *parent) : QDateTimeEdit(parent)
{
    setCalendarPopup(true);

    auto *cw = new CustomCalendarWidget(this);
    setCalendarWidget(cw);

    setDisplayFormat(QStringLiteral("dd.MM.yy HH:mm"));
    setSpecialValueText(QStringLiteral("NULL"));

    connect(lineEdit(), &QLineEdit::textChanged,
            this, &RestDateTimeEdit::txtChangeSlot);
    connect(cw, &CustomCalendarWidget::shown,
            this, &RestDateTimeEdit::onCalendarShown);
}

void RestDateTimeEdit::setDateTime(const QDateTime &dateTime)
{
    QDateTimeEdit::setDateTime(dateTime.isNull() ? minimumDateTime() : dateTime);
}

void RestDateTimeEdit::clear()
{
    QDateTimeEdit::setDateTime(minimumDateTime());
}

void RestDateTimeEdit::txtChangeSlot(const QString &txt)
{
    if (txt.isEmpty()) {
        blockSignals(true);
        setDateTime(minimumDateTime());
        blockSignals(false);
    }
}

void RestDateTimeEdit::onCalendarShown()
{
    if (dateTime() == minimumDateTime()) {
        setDateTime(QDateTime::currentDateTime());
    }
}