#ifndef RESTDATEEDIT_H
#define RESTDATEEDIT_H

#include <QDateEdit>
#include <QCalendarWidget>
#include <QShowEvent>
#include <QLineEdit>

class CustomCalendarWidget : public QCalendarWidget
{
    Q_OBJECT
public:
    CustomCalendarWidget(QWidget *parent = nullptr);
    virtual void showEvent(QShowEvent *event);
signals:
    void shown();
};

class RestDateEdit : public QDateEdit
{
    Q_OBJECT
public:
    RestDateEdit(QWidget *parent = nullptr);

public slots:
    void setDate(QDate date);
    void clear();

private slots:
    void txtChangeSlot(QString txt);
    void shVid();
};

class RestDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    RestDateTimeEdit(QWidget *parent = nullptr);

public slots:
    void setDateTime(const QDateTime &dateTime);
    void clear();

private slots:
    void txtChangeSlot(QString txt);
    void shVid();
};

#endif // RESTDATEEDIT_H
