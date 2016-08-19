#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "bincomp.h"
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
    void refreshDataViews();

    void closeEvent(QCloseEvent* event);

    QScopedPointer<DebugWindow> m_DebugWindow;

    //QVector<unsigned char> dataSet1;
    //QVector<unsigned char> dataSet2;
    //dataSet dataSet1;
    dataSet dataSet2;

    QSharedPointer<dataSet> m_dataSet1;

    QSharedPointer<dataSetView> m_dataSetView1;

    QSharedPointer<QVector<byterange>> m_diffs;
    //byterange subset;

    void doCompare();

};

#endif // MAINWINDOW_H
