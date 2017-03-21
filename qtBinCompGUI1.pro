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
    hexfield.cpp \
    log.cpp \
    settingsdialog.cpp \
    usersettings.cpp \
    comparison.cpp \
    rollinghashcpp/mersennetwister.cpp \
    blockmatchset.cpp \
    comparisonthread.cpp

HEADERS  += mainwindow.h \
    dataSet.h \
    dataSetView.h \
    debugwindow.h \
    hexfield.h \
    byteRange.h \
    log.h \
    settingsdialog.h \
    usersettings.h \
    defensivecoding.h \
    comparison.h \
    rollinghashcpp/characterhash.h \
    rollinghashcpp/cyclichash.h \
    rollinghashcpp/generalhash.h \
    rollinghashcpp/mersennetwister.h \
    rollinghashcpp/rabinkarphash.h \
    rollinghashcpp/threewisehash.h \
    rollinghashcpp/ztimer.h \
    blockmatchset.h \
    comparisonthread.h

FORMS    += mainwindow.ui \
    debugwindow.ui \
    settingsdialog.ui

DISTFILES += \
    log

SUBDIRS += \
    qtBinCompGUI1_Test.pro \
    qtBinCompGUI1_Test.pro
