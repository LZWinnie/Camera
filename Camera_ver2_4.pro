#-------------------------------------------------
#
# Project created by QtCreator 2018-09-07T16:21:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Camera_ver2_4
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    imgpro.cpp \
    help.cpp

HEADERS += \
        mainwindow.h \
    avglobal.h \
    dhconfigsdk.h \
    dhnetsdk.h \
    imgpro.h \
    help.h

FORMS += \
        mainwindow.ui \
    help.ui

INCLUDEPATH += D:\Program\opencv\build\include \
            D:\Program\opencv_contrib\include

LIBS += \
        D:\Program\opencv\build\x64\vc14\lib\*.lib \
        D:\Program\opencv_contrib\x64\vc14\lib\*.lib \
        D:\Qt\NetSDK_Chn_Bin64\lib\*.lib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    help.qrc
