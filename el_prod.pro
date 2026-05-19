QT += widgets network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    formpart/formpart.cpp \
    formreport/formreport.cpp \
    main.cpp \
    mainwindow.cpp \
    progressreportdialog/progressreportdialog.cpp \
    rest/httpsyncmanager.cpp \
    rest/relmodels.cpp \
    rest/restcombobox.cpp \
    rest/restconnection.cpp \
    rest/restdateedit.cpp \
    rest/restitemdelegate.cpp \
    rest/restlogin.cpp \
    rest/restmapper.cpp \
    rest/restrelmodel.cpp \
    rest/restrotablemodel.cpp \
    rest/resttablemodel.cpp \
    rest/resttableview.cpp \
    tabmanager.cpp

HEADERS += \
    formpart/formpart.h \
    formreport/formreport.h \
    mainwindow.h \
    progressreportdialog/progressreportdialog.h \
    rest/httpsyncmanager.h \
    rest/relmodels.h \
    rest/restcombobox.h \
    rest/restconnection.h \
    rest/restdateedit.h \
    rest/restitemdelegate.h \
    rest/restlogin.h \
    rest/restmapper.h \
    rest/restrelmodel.h \
    rest/restrotablemodel.h \
    rest/resttablemodel.h \
    rest/resttableview.h \
    tabmanager.h

FORMS += \
    formpart/formpart.ui \
    formreport/formreport.ui \
    mainwindow.ui \
    progressreportdialog/progressreportdialog.ui \
    rest/restlogin.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
