#include "mainwindow.h"

#include <QApplication>
#include "rest/restlogin.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/ico.ico"));
    RestLogin d(QObject::tr("Производство электродов"));
    d.setHost("https://192.168.1.10");
    if (d.exec()!=QDialog::Accepted) {
        exit(1);
    }
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
