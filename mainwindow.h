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

#include "byteRange.h"
#include "dataSet.h"
#include "dataSetView.h"
#include "settingsdialog.h"
#include "usersettings.h"
#include "defensivecoding.h"
#include "comparison.h"
#include "comparisonthread.h"
#include "offsetmetrics.h"

#include <set>

namespace Ui {
class MainWindow;
}

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

private:
    Ui::MainWindow *ui;
    void doScrollBar(int value);
    void doLoadFile1(const QString filename);
    void doLoadFile2(const QString filename);
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

    void doCompare();
    void doSimpleCompare();
    void displayLogMessage(QString str, QColor color);

    UserSettings m_userSettings;

    comparisonThread m_comparisonThread;

stopwatch STOPWATCH1;
bool DEBUGFLAG1 = false;
};

#endif // MAINWINDOW_H
