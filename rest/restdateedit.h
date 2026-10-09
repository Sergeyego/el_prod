#ifndef RESTDATEEDIT_H
#define RESTDATEEDIT_H

#include <QDateEdit>
#include <QDateTimeEdit>
#include <QCalendarWidget>
#include <QShowEvent>
#include <QLineEdit>

class CustomCalendarWidget : public QCalendarWidget
{
    Q_OBJECT
public:
    explicit CustomCalendarWidget(QWidget *parent = nullptr);

signals:
    void shown();

protected:
    void showEvent(QShowEvent *event) override;
};

class RestDateEdit : public QDateEdit
{
    Q_OBJECT
public:
    explicit RestDateEdit(QWidget *parent = nullptr);

public slots:
    void setDate(const QDate &date);
    void clear() override;

private slots:
    void txtChangeSlot(const QString &txt);
    void onCalendarShown();
};

class RestDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    explicit RestDateTimeEdit(QWidget *parent = nullptr);

public slots:
    void setDateTime(const QDateTime &dateTime);
    void clear() override;

private slots:
    void txtChangeSlot(const QString &txt);
    void onCalendarShown();
};

#endif // RESTDATEEDIT_H
