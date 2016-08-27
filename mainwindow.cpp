#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QVector>
#include <QString>
#include <QStringBuilder>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "debugwindow.h"

#include "byteRange.h"
#include "dataSet.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_DebugWindow(new DebugWindow())
{
    ui->setupUi(this);

    //create dataSets
    m_dataSet1 = QSharedPointer<dataSet>::create();
    m_dataSet2 = QSharedPointer<dataSet>::create();

    connect(m_dataSet1.data(), &dataSet::sizeChanged, m_DebugWindow.data(), &DebugWindow::dataSet1SizeChanged);
    connect(m_dataSet2.data(), &dataSet::sizeChanged, m_DebugWindow.data(), &DebugWindow::dataSet2SizeChanged);

    connect(m_dataSet1.data(), &dataSet::sizeChanged, this, &MainWindow::refreshDataViews);
    connect(m_dataSet2.data(), &dataSet::sizeChanged, this, &MainWindow::refreshDataViews);

    connect(ui->verticalScrollBar, &QScrollBar::valueChanged, this, &MainWindow::doScrollBar);

    connect(ui->textEdit_dataSet1, &hexField::filenameDropped, this, &MainWindow::doLoadFile1);
    connect(ui->textEdit_dataSet2, &hexField::filenameDropped, this, &MainWindow::doLoadFile2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::doScrollBar(int value)
{
    if (m_dataSetView1.isNull() || m_dataSetView2.isNull()) {
        return;
    }

    m_dataSetView1->setSubsetStart(value);
    m_dataSetView2->setSubsetStart(value);

    refreshDataViews();
}

void MainWindow::refreshDataViews()
{
    if (m_dataSet1.isNull() || m_dataSetView1.isNull() ||
        m_dataSet2.isNull() || m_dataSetView2.isNull()) {
        return;
    }

    ui->textEdit_dataSet1->clear();
    m_dataSetView1->vectorSubsetToQTextEdit(ui->textEdit_dataSet1);

    ui->textEdit_dataSet2->clear();
    m_dataSetView2->vectorSubsetToQTextEdit(ui->textEdit_dataSet2);
}

void MainWindow::on_actionShow_Debug_triggered()
{
    m_DebugWindow->show();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    m_DebugWindow->close();
    QMainWindow::closeEvent(event);
}

void MainWindow::on_actionTest_triggered()
{
    ui->textEdit_dataSet1->clear();
    ui->textEdit_dataSet2->clear();

    doLoadFile1("test1");
    doLoadFile2("test2");

    doCompare();
}

void MainWindow::doLoadFile1(const QString filename)
{
    dataSet::loadFileResult res = m_dataSet1->loadFile(filename);
    if      (res == dataSet::loadFileResult::ERROR_FileDoesNotExist) {
        QMessageBox::warning(this,"Error", "File \"" % filename % "\" does not exist.");
    }
    else if (res == dataSet::loadFileResult::ERROR_FileReadFailure) {
        QMessageBox::warning(this,"Error", "\"" % filename % "\" failed to read.");
    }
}

void MainWindow::doLoadFile2(const QString filename)
{
    dataSet::loadFileResult res = m_dataSet2->loadFile(filename);
    if      (res == dataSet::loadFileResult::ERROR_FileDoesNotExist) {
        QMessageBox::warning(this,"Error", "File \"" % filename % "\" does not exist.");
    }
    else if (res == dataSet::loadFileResult::ERROR_FileReadFailure) {
        QMessageBox::warning(this,"Error", "\"" % filename % "\" failed to read.");
    }
}

void MainWindow::doCompare()
{

    m_diffs = QSharedPointer<QVector<byteRange>>::create();
    dataSet::compare(*m_dataSet1.data(), *m_dataSet2.data(), *m_diffs.data());

    m_dataSetView1 = QSharedPointer<dataSetView>::create(m_dataSet1, m_diffs);
    m_dataSetView1->setSubset(byteRange(0,128));

    m_dataSetView2 = QSharedPointer<dataSetView>::create(m_dataSet2, m_diffs);
    m_dataSetView2->setSubset(byteRange(0,128));

    m_dataSetView1->vectorSubsetToQTextEdit(ui->textEdit_dataSet1);
    m_dataSetView2->vectorSubsetToQTextEdit(ui->textEdit_dataSet2);

    connect(m_dataSetView1.data(), &dataSetView::subsetChanged, m_DebugWindow.data(), &DebugWindow::dataSet1RangeChanged);
    connect(m_dataSetView2.data(), &dataSetView::subsetChanged, m_DebugWindow.data(), &DebugWindow::dataSet2RangeChanged);

    //bug for data < subset count, max can go negative
    ui->verticalScrollBar->setMinimum(0);
    ui->verticalScrollBar->setMaximum(m_dataSet1->getData()->size() - m_dataSetView1->getSubset().count);

    emit ui->verticalScrollBar->valueChanged(ui->verticalScrollBar->value());   //fire signal for current scroll bar position
}


void MainWindow::on_B_Compare_clicked()
{
    doCompare();
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionLoad_File1_Left_triggered()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, "Load File 1 (Left)");
    doLoadFile1(filename);
}

void MainWindow::on_actionLoad_File2_Right_triggered()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, "Load File 2 (Left)");
    doLoadFile2(filename);
}
