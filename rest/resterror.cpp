#include "resterror.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QDateTime>
#include <QLabel>
#include <QStyle>
#include <QTextEdit>
#include <QTextCursor>
#include <QTextBlockFormat>
#include <QTextFormat>
#include <QTextBlock>
#include <QDebug>

RestError *RestError::instance()
{
    static RestError obj;
    return &obj;
}

RestError::RestError(QObject *parent)
    : QObject(parent)
{
}

RestError::~RestError()
{
    if (m_dialog) {
        m_dialog->close();
    }
}

void RestError::push(const QString &errorTitle, const QString &errorText)
{
    const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // Экранируем HTML-спецсимволы, а затем заменяем переносы строк на <br>
    const QString escapedTitle = errorTitle.toHtmlEscaped();
    const QString escapedText  = errorText.toHtmlEscaped().replace("\n", "<br>");

    const QString formattedError = QString("[%1] <b>%2</b><br>%3")
                                       .arg(timestamp, escapedTitle, escapedText);

    //qCritical().noquote() << QString("[%1] %2: %3").arg(timestamp, errorTitle, errorText);

    m_errors.append(formattedError);
    while (m_errors.size() > kMaxErrors) {
        m_errors.removeFirst();
    }

    if (m_dialog && m_dialog->isVisible() && m_textDisplay) {
        appendErrorHtml(formattedError, m_errors.size() > 1);
    } else {
        showDialog();
    }
}

void RestError::clear()
{
    m_errors.clear();
    if (m_textDisplay) {
        m_textDisplay->clear();
    }
}

void RestError::showDialog()
{
    QWidget *parentWidget = QApplication::activeWindow();

    m_dialog = new QDialog(parentWidget);
    m_dialog->setWindowTitle(tr("Ошибка"));
    m_dialog->resize(550, 200);
    m_dialog->setWindowModality(Qt::NonModal);
    m_dialog->setAttribute(Qt::WA_DeleteOnClose);
    m_dialog->setWindowFlags(m_dialog->windowFlags() | Qt::WindowStaysOnTopHint);

    const QIcon criticalIcon = m_dialog->style()->standardIcon(QStyle::SP_MessageBoxCritical);
    m_dialog->setWindowIcon(criticalIcon);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_dialog);
    QHBoxLayout *contentLayout = new QHBoxLayout();

    QLabel *iconLabel = new QLabel(m_dialog);
    const int iconSize = m_dialog->style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
    iconLabel->setPixmap(criticalIcon.pixmap(iconSize, iconSize));
    iconLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_textDisplay = new QTextEdit(m_dialog);
    m_textDisplay->setReadOnly(true);
    m_textDisplay->setLineWrapMode(QTextEdit::WidgetWidth);
    // Убираем рамку QTextEdit, так как она дублирует рамку диалога
    m_textDisplay->setFrameStyle(QFrame::NoFrame);

    contentLayout->addWidget(iconLabel);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(m_textDisplay, 1);
    mainLayout->addLayout(contentLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton *btnOk = new QPushButton(tr("Закрыть"), m_dialog);
    btnOk->setAutoDefault(true);
    buttonLayout->addWidget(btnOk);
    mainLayout->addLayout(buttonLayout);

    connect(btnOk, &QPushButton::clicked, m_dialog, &QDialog::accept);
    connect(m_dialog, &QDialog::finished, this, &RestError::onDialogClosed);

    updateDialogText();

    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void RestError::updateDialogText()
{
    if (!m_textDisplay) return;

    m_textDisplay->clear();
    for (int i = 0; i < m_errors.size(); ++i) {
        appendErrorHtml(m_errors[i], i > 0);
    }
}

void RestError::appendErrorHtml(const QString &html, bool withSeparator)
{
    if (!m_textDisplay) return;

    QTextCursor cursor = m_textDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);

    if (withSeparator) {
        // Завершаем предыдущий блок с текстом
        if (!cursor.block().text().isEmpty()) {
            cursor.insertBlock();
        }

        // Текущему пустому блоку задаем линию снизу
        QTextBlockFormat lineFmt;
        lineFmt.setTopMargin(10);
        lineFmt.setBottomMargin(10);
        lineFmt.setProperty(QTextFormat::BlockTrailingHorizontalRulerWidth, 100);
        cursor.setBlockFormat(lineFmt);

        // Создаем новый блок для текста и сразу сбрасываем формат,
        // чтобы линия не наследовалась на строки ниже
        cursor.insertBlock();
        QTextBlockFormat normalFmt; // пустой формат по умолчанию
        cursor.setBlockFormat(normalFmt);
    }

    // Вставляем сам текст ошибки
    cursor.insertHtml(html);

    m_textDisplay->setTextCursor(cursor);
    m_textDisplay->ensureCursorVisible();
}

void RestError::onDialogClosed()
{
    clear();
}