#-------------------------------------------------
#
# Project created by QtCreator 2016-07-25T15:49:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtBinCompGUI1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dataSet.cpp \
    dataSetView.cpp \
    debugwindow.cpp \
    hexfield.cpp

HEADERS  += mainwindow.h \
    dataSet.h \
    dataSetView.h \
    debugwindow.h \
    hexfield.h \
    byteRange.h

FORMS    += mainwindow.ui \
    debugwindow.ui

DISTFILES += \
    todo

SUBDIRS += \
    qtBinCompGUI1_Test.pro
