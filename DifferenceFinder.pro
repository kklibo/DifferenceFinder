#-------------------------------------------------
#
# Project created by QtCreator 2016-07-25T15:49:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DifferenceFinder
TEMPLATE = app


SOURCES += main.cpp\
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
    buzhash.cpp

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
    buzhash.h

FORMS    += mainwindow.ui \
    settingsdialog.ui

DISTFILES += \
    log \
    README.md
