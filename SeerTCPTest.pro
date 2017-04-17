#-------------------------------------------------
#
# Project created by QtCreator 2017-04-14T13:49:57
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SeerTCPTest
TEMPLATE = app


SOURCES += main.cpp\
        SCTcpToolWidget.cpp \
    SCStatusTcp.cpp

HEADERS  += SCTcpToolWidget.h \
    SCStatusTcp.h \
    SCHeadData.h

FORMS    += SCTcpToolWidget.ui

RC_FILE = ICO.rc
