#include "restdateedit.h"

RestDateEdit::RestDateEdit(QWidget *parent) : QDateEdit(parent)
{
    this->setCalendarPopup(true);
    CustomCalendarWidget * pCW = new CustomCalendarWidget(this);
    this->setCalendarWidget(pCW);
    this->setDisplayFormat("dd.MM.yy");
    this->setSpecialValueText("NULL");
    connect(this->lineEdit(),SIGNAL(textChanged(QString)),this,SLOT(txtChangeSlot(QString)));
    connect(pCW,SIGNAL(shown()),this,SLOT(shVid()));

}

void RestDateEdit::setDate(QDate date)
{
    QDateEdit::setDate(date.isNull()? this->minimumDate() : date);
}

void RestDateEdit::clear()
{
    QDateEdit::setDate(this->minimumDate());
}

void RestDateEdit::txtChangeSlot(QString txt)
{
    if (txt.isEmpty()){
        this->setDate(this->minimumDate());
    }
}

void RestDateEdit::shVid()
{
    if (this->date()==this->minimumDate()){
        this->setDate(QDate::currentDate());
    }
}

RestDateTimeEdit::RestDateTimeEdit(QWidget *parent) : QDateTimeEdit(parent)
{
    this->setCalendarPopup(true);
    CustomCalendarWidget * pCW = new CustomCalendarWidget(this);
    this->setCalendarWidget(pCW);
    this->setDisplayFormat("dd.MM.yyyy hh:mm");
    this->setSpecialValueText("NULL");
    connect(this->lineEdit(),SIGNAL(textChanged(QString)),this,SLOT(txtChangeSlot(QString)));
    connect(pCW,SIGNAL(shown()),this,SLOT(shVid()));
}

void RestDateTimeEdit::setDateTime(const QDateTime &dateTime)
{
    QDateTimeEdit::setDateTime(dateTime.isNull()? this->minimumDateTime() : dateTime);
}

void RestDateTimeEdit::clear()
{
    QDateTimeEdit::setDateTime(this->minimumDateTime());
}

void RestDateTimeEdit::txtChangeSlot(QString txt)
{
    if (txt.isEmpty()){
        this->setDateTime(this->minimumDateTime());
    }
}

void RestDateTimeEdit::shVid()
{
    if (this->dateTime()==this->minimumDateTime()){
        this->setDateTime(QDateTime::currentDateTime());
    }
}

CustomCalendarWidget::CustomCalendarWidget(QWidget *parent) : QCalendarWidget(parent)
{
    this->setFirstDayOfWeek(Qt::Monday);
}

void CustomCalendarWidget::showEvent(QShowEvent *event)
{
    if (event->type() == QEvent::Show){
        emit shown();
    }
    QCalendarWidget::showEvent(event);
}
