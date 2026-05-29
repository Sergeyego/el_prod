QT += widgets network printsupport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogloadchem/dialogloadchem.cpp \
    dialogpackload/dialogpackload.cpp \
    formfix/formfix.cpp \
    formpack/formpack.cpp \
    formpart/formpart.cpp \
    formperepack/formperepack.cpp \
    formreport/formreport.cpp \
    formself/formself.cpp \
    invoicemanager.cpp \
    main.cpp \
    mainwindow.cpp \
    progressreportdialog/progressreportdialog.cpp \
    rest/relmodels.cpp \
    rest/restcombobox.cpp \
    rest/restconnection.cpp \
    rest/restdateedit.cpp \
    rest/restitemdelegate.cpp \
    rest/restlogin.cpp \
    rest/restmapper.cpp \
    rest/restrelmodel.cpp \
    rest/restrotablemodel.cpp \
    rest/resttabledialog.cpp \
    rest/resttablemodel.cpp \
    rest/resttableview.cpp \
    rest_olap/axiswidget.cpp \
    rest_olap/cubewidget.cpp \
    rest_olap/cubic.cpp \
    rest_olap/dialogolapflt.cpp \
    rest_olap/formfilter.cpp \
    rest_olap/olapmodel.cpp \
    tabmanager.cpp

HEADERS += \
    dialogloadchem/dialogloadchem.h \
    dialogpackload/dialogpackload.h \
    formfix/formfix.h \
    formpack/formpack.h \
    formpart/formpart.h \
    formperepack/formperepack.h \
    formreport/formreport.h \
    formself/formself.h \
    invoicemanager.h \
    mainwindow.h \
    progressreportdialog/progressreportdialog.h \
    rest/relmodels.h \
    rest/restcombobox.h \
    rest/restconnection.h \
    rest/restdateedit.h \
    rest/restitemdelegate.h \
    rest/restlogin.h \
    rest/restmapper.h \
    rest/restrelmodel.h \
    rest/restrotablemodel.h \
    rest/resttabledialog.h \
    rest/resttablemodel.h \
    rest/resttableview.h \
    rest_olap/axiswidget.h \
    rest_olap/cubewidget.h \
    rest_olap/cubic.h \
    rest_olap/dialogolapflt.h \
    rest_olap/formfilter.h \
    rest_olap/olapmodel.h \
    tabmanager.h

FORMS += \
    dialogloadchem/dialogloadchem.ui \
    dialogpackload/dialogpackload.ui \
    formfix/formfix.ui \
    formpack/formpack.ui \
    formpart/formpart.ui \
    formperepack/formperepack.ui \
    formreport/formreport.ui \
    formself/formself.ui \
    mainwindow.ui \
    progressreportdialog/progressreportdialog.ui \
    rest/restlogin.ui \
    rest/resttabledialog.ui \
    rest_olap/axiswidget.ui \
    rest_olap/cubewidget.ui \
    rest_olap/dialogolapflt.ui \
    rest_olap/formfilter.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:RC_FILE = ico.rc
macx:ICON = ico.icns

RESOURCES += \
    res.qrc

DISTFILES += \
    ico.rc
