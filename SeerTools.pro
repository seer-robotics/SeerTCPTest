#-------------------------------------------------
#
# Project created by QtCreator 2017-04-14T13:49:57
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11

TARGET = SeerTCPTest
TEMPLATE = app
include(libconfig.prf)

SOURCES += main.cpp\
        SCTcpToolWidget.cpp \
    SCStatusTcp.cpp \
    Core/BaseThread.cpp \
    Core/SqliteClass.cpp

HEADERS  += SCTcpToolWidget.h \
    SCStatusTcp.h \
    SCHeadData.h \
    Core/BaseThread.h \
    Core/SqliteClass.h

FORMS   += SCTcpToolWidget.ui

win32:RC_FILE = ICO.rc

RESOURCES += \
    resource.qrc
