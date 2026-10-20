#ifndef RESTERROR_H
#define RESTERROR_H

#include <QObject>
#include <QStringList>
#include <QPointer>
#include <QDialog>

class QTextEdit;

class RestError : public QObject
{
    Q_OBJECT
public:
    static RestError *instance();

    static void critical(const QString &title, const QString &text) {
        instance()->push(title, text);
    }

    void push(const QString &errorTitle, const QString &errorText);
    void clear();

private:
    explicit RestError(QObject *parent = nullptr);
    ~RestError() override;

    RestError(const RestError&) = delete;
    RestError& operator=(const RestError&) = delete;

    void showDialog();
    void updateDialogText();
    void appendErrorHtml(const QString &html, bool withSeparator);

    QStringList m_errors;
    QPointer<QDialog> m_dialog;
    QPointer<QTextEdit> m_textDisplay;

    static constexpr int kMaxErrors = 200;

private slots:
    void onDialogClosed();
};

#endif // RESTERROR_H