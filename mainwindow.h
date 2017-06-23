#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QVector>
#include <QString>
#include <QStringBuilder>
#include <QtGlobal>

#include "log.h"
#include "ui_mainwindow.h"

#include "indexrange.h"
#include "dataSet.h"
#include "dataSetView.h"
#include "settingsdialog.h"
#include "usersettings.h"
#include "defensivecoding.h"
#include "comparison.h"
#include "comparisonthread.h"
#include "offsetmetrics.h"
#include "utilities.h"
#include "searchprocessing.h"

#include <set>

namespace Ui {
class MainWindow;
}


class scrollWheelRedirector : public QObject
{
//used to redirect QWheelEvents to the main scrollbar
    Q_OBJECT

public:
    scrollWheelRedirector(QObject* redirectTo);

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    QObject* m_redirectTo;
};


class scrollBarKeyFilter : public QObject
{
//makes certain keypresses (from anywhere in the application)
//  control the scrollbar
    Q_OBJECT

public:
    scrollBarKeyFilter(QScrollBar* scrollBar);

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    QScrollBar* m_QScrollBar;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void onComparisonThreadEnded();


    void on_actionQuit_triggered();

    void on_actionLoad_File1_Left_triggered();

    void on_actionLoad_File2_Right_triggered();

    void onHexFieldFontChange();

    void on_actionSettings_triggered();

    void on_actionTest_Compare3_triggered();

    void on_actionDoSimpleCompare_triggered();

    void on_actionStop_thread_triggered();

    void on_actionTest_triggered();

    void on_actionDebugFlag_triggered();

    void on_actionSequential_compare_triggered();

    void on_actionLargestBlock_compare_triggered();

    void on_actionSwitch_files_triggered();

    void on_actionTest_load_triggered();

private:

    QSharedPointer<scrollWheelRedirector> m_scrollWheelRedirector;
    QSharedPointer<scrollBarKeyFilter> m_scrollBarKeyFilter;

    Ui::MainWindow *ui;
    void doScrollBar(int value);

    void doLoadFile1(const QString filename);
    void doLoadFile1FromMemory(std::unique_ptr<std::vector<unsigned char>> data);
    void updateUIforFile1Load();

    void doLoadFile2(const QString filename);
    void doLoadFile2FromMemory(std::unique_ptr<std::vector<unsigned char>> data);
    void updateUIforFile2Load();

    void updateScrollBarRange();
    void resizeHexField1();
    void resizeHexField2();
    void applyUserSettingsTo(QSharedPointer<dataSetView> ds);
    void refreshTitleBarText();


    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

    //application title string
    static const QString APPLICATION_TITLE;

    //the 2 dataSets we will compare
    QSharedPointer<dataSet> m_dataSet1;
    QSharedPointer<dataSet> m_dataSet2;

    //these will use the QT interface to display the dataSets
    QSharedPointer<dataSetView> m_dataSetView1;
    QSharedPointer<dataSetView> m_dataSetView2;

    void doSimpleCompare();
    void displayLogMessage(QString str, QColor color);

    static QString summarizeResults(const    comparison::results& results);
    static QString summarizeResults(const offsetMetrics::results& results);

    UserSettings m_userSettings;

    comparisonThread m_comparisonThread;

stopwatch STOPWATCH1;
bool DEBUGFLAG1 = false;
};

#endif // MAINWINDOW_H
