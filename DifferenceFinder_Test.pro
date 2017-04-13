#-------------------------------------------------
#
# Project created by QtCreator 2016-07-25T15:49:52
#  copied 2016.8.24; modified for google test
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DifferenceFinder
TEMPLATE = app


#SOURCES += main.cpp\
SOURCES +=\
        mainwindow.cpp \
    dataSet.cpp \
    dataSetView.cpp \
    hexfield.cpp \
    log.cpp \
    settingsdialog.cpp \
    usersettings.cpp \
    comparison.cpp \
    blockmatchset.cpp \
    comparisonthread.cpp \
    stopwatch.cpp \
    buzhash.cpp \
    dataSetTest.cpp

HEADERS  += mainwindow.h \
    dataSet.h \
    dataSetView.h \
    hexfield.h \
    byteRange.h \
    log.h \
    settingsdialog.h \
    usersettings.h \
    defensivecoding.h \
    comparison.h \
    blockmatchset.h \
    comparisonthread.h \
    stopwatch.h \
    buzhash.h \
    gtestDefs.h

FORMS    += mainwindow.ui \
    settingsdialog.ui

DISTFILES += \
    log \
    README.md

INCLUDEPATH += \
    /usr/include/gtest \
    /usr/include/gmock

LIBS += -lgtest -L/usr/lib
LIBS += -lgmock -L/usr/lib
LIBS += -lgtest_main -L/usr/lib
LIBS += -lgmock_main -L/usr/lib
