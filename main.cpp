#include "mainwindow.h"

#include <QApplication>
#include "rest/restlogin.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RestLogin d(QObject::tr("Производство электродов"));
    if (d.exec()!=QDialog::Accepted) {
        exit(1);
    }
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
