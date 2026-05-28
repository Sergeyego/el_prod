#ifndef INVOICEMANAGER_H
#define INVOICEMANAGER_H

#include <QObject>
#include <QNetworkReply>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QRegularExpression>
#include "rest/restconnection.h"

class InvoiceManager : public QObject
{
    Q_OBJECT
public:
    explicit InvoiceManager(QObject *parent = nullptr);
    void getInvoice(QString path, QString vid, QString type, QString filename, int year);

private slots:
    void onResult();
};

#endif // INVOICEMANAGER_H
