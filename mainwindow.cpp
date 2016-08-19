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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "debugwindow.h"

#include "bincomp.h"
#include "dataSet.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_DebugWindow(new DebugWindow())
{
    ui->setupUi(this);

    m_dataSet1 = QSharedPointer<dataSet>::create();
    m_dataSet2 = QSharedPointer<dataSet>::create();

    connect(m_dataSet1.data(), &dataSet::sizeChanged, m_DebugWindow.data(), &DebugWindow::dataSet1SizeChanged);
    connect(m_dataSet2.data(), &dataSet::sizeChanged, m_DebugWindow.data(), &DebugWindow::dataSet2SizeChanged);

    connect(m_dataSet1.data(), &dataSet::sizeChanged, this, &MainWindow::refreshDataViews);
    connect(m_dataSet2.data(), &dataSet::sizeChanged, this, &MainWindow::refreshDataViews);

    connect(ui->verticalScrollBar, &QScrollBar::valueChanged, this, &MainWindow::doScrollBar);

    connect(ui->textEdit_dataSet1, &hexField::filenameDropped, m_dataSet1.data(), &dataSet::loadFile);
    connect(ui->textEdit_dataSet2, &hexField::filenameDropped, m_dataSet2.data(), &dataSet::loadFile);
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

    if (m_dataSet1->getData()->size()) {
        ui->textEdit_dataSet1->clear();
        m_dataSetView1->vectorSubsetToQTextEdit(ui->textEdit_dataSet1);
    }

    if (m_dataSet2->getData()->size()) {
        ui->textEdit_dataSet2->clear();
        m_dataSetView2->vectorSubsetToQTextEdit(ui->textEdit_dataSet2);
    }
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

    m_dataSet1->loadFile("test1");
    m_dataSet2->loadFile("test2");

    doCompare();
}

void MainWindow::doCompare()
{

    m_diffs = QSharedPointer<QVector<byterange>>::create();
    dataSet::compare(*m_dataSet1.data(), *m_dataSet2.data(), *m_diffs.data());

    m_dataSetView1 = QSharedPointer<dataSetView>::create(m_dataSet1, m_diffs);
    m_dataSetView1->setSubset(byterange(4,128));

    m_dataSetView2 = QSharedPointer<dataSetView>::create(m_dataSet2, m_diffs);
    m_dataSetView2->setSubset(byterange(4,128));

    m_dataSetView1->vectorSubsetToQTextEdit(ui->textEdit_dataSet1);
    m_dataSetView2->vectorSubsetToQTextEdit(ui->textEdit_dataSet2);

    connect(m_dataSetView1.data(), &dataSetView::subsetChanged, m_DebugWindow.data(), &DebugWindow::dataSet1RangeChanged);
    connect(m_dataSetView2.data(), &dataSetView::subsetChanged, m_DebugWindow.data(), &DebugWindow::dataSet2RangeChanged);

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
    if (!filename.isEmpty()) {
        this->m_dataSet1->loadFile(filename);
    }
}

void MainWindow::on_actionLoad_File2_Right_triggered()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, "Load File 2 (Left)");
    if (!filename.isEmpty()) {
        this->m_dataSet2->loadFile(filename);
    }
}
