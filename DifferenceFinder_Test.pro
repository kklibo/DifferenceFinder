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
SOURCES +=\
        mainwindow.cpp \
    dataSet.cpp \
    dataSetView.cpp \
    debugwindow.cpp \
    hexfield.cpp \
    log.cpp \
    dataSetTest.cpp

HEADERS  += mainwindow.h \
    dataSet.h \
    dataSetView.h \
    debugwindow.h \
    hexfield.h \
    byteRange.h \
    log.h \
    gtestDefs.h

FORMS    += mainwindow.ui \
    debugwindow.ui

DISTFILES += \
    todo \
    log

INCLUDEPATH += \
    /usr/include/gtest \
    /usr/include/gmock

LIBS += -lgtest -L/usr/lib
LIBS += -lgmock -L/usr/lib
LIBS += -lgtest_main -L/usr/lib
LIBS += -lgmock_main -L/usr/lib
