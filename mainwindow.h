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
#include "debugwindow.h"

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
    void on_actionShow_Debug_triggered();

    void on_actionTest_triggered();

    void on_B_Compare_clicked();

    void on_actionQuit_triggered();

    void on_actionLoad_File1_Left_triggered();

    void on_actionLoad_File2_Right_triggered();

private:
    Ui::MainWindow *ui;
    void doScrollBar(int value);
    void doLoadFile1(const QString filename);
    void doLoadFile2(const QString filename);
    void refreshDataViews();

    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

    QScopedPointer<DebugWindow> m_DebugWindow;

    //the 2 dataSets we will compare
    QSharedPointer<dataSet> m_dataSet1;
    QSharedPointer<dataSet> m_dataSet2;

    //these will use the QT interface to display the dataSets
    QSharedPointer<dataSetView> m_dataSetView1;
    QSharedPointer<dataSetView> m_dataSetView2;

    //differences between the dataSets
    QSharedPointer<QVector<byteRange>> m_diffs;

    void doCompare();
    void displayLogMessage(QString str, QColor color);


};

#endif // MAINWINDOW_H
