#-------------------------------------------------
#
# Project created by QtCreator 2016-07-25T15:49:52
#  copied 2016.8.24; modified for google test
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtBinCompGUI1
TEMPLATE = app


#SOURCES += main.cpp\
SOURCES += gtest.cpp\
        mainwindow.cpp \
    dataSet.cpp \
    dataSetView.cpp \
    debugwindow.cpp \
    hexfield.cpp \
    dataSetTest.cpp

HEADERS  += mainwindow.h \
    bincomp.h \
    dataSet.h \
    dataSetView.h \
    debugwindow.h \
    hexfield.h

FORMS    += mainwindow.ui \
    debugwindow.ui

DISTFILES += \
    todo

INCLUDEPATH += \
    /usr/include/gtest \
    /usr/include/gmock

LIBS += -lgtest -L/usr/lib
LIBS += -lgmock -L/usr/lib
LIBS += -lgtest_main -L/usr/lib
LIBS += -lgmock_main -L/usr/lib
