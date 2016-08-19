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

    connect(m_dataSet1.data(), &dataSet::sizeChanged, m_DebugWindow.data(), &DebugWindow::dataSet1SizeChanged);
    connect(  &dataSet2      , &dataSet::sizeChanged, m_DebugWindow.data(), &DebugWindow::dataSet2SizeChanged);

    connect(m_dataSet1.data(), &dataSet::sizeChanged, this, &MainWindow::refreshDataViews);
    connect(  &dataSet2      , &dataSet::sizeChanged, this, &MainWindow::refreshDataViews);

    connect(ui->verticalScrollBar, &QScrollBar::valueChanged, this, &MainWindow::doScrollBar);

    connect(ui->textEdit_dataSet1, &hexField::filenameDropped, m_dataSet1.data(), &dataSet::loadFile);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::doScrollBar(int value)
{
    ui->textEdit_dataSet1->setText("testSlot");

    if (m_dataSetView1.isNull()) {
        return;
    }

    m_dataSetView1->setSubsetStart(value);

    refreshDataViews();
}

void MainWindow::refreshDataViews()
{
    if (m_dataSet1.isNull() || m_dataSetView1.isNull()) {
        return;
    }

    if (m_dataSet1->getData()->size()) {
        m_dataSetView1->vectorSubsetToQTextEdit(ui->textEdit_dataSet1);
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

    //fileToVector("test1", dataSet1);
    //fileToVector("test2", dataSet2);


//    m_dataSet1 = QSharedPointer<dataSet>::create();

    //connect(m_dataSet1.data(), &dataSet::sizeChanged, m_DebugWindow.data(), &DebugWindow::dataSet1SizeChanged);
//    connect(  &dataSet2      , &dataSet::sizeChanged, m_DebugWindow.data(), &DebugWindow::dataSet2SizeChanged);

    m_dataSet1->loadFile("test1");
    dataSet2.loadFile("test2");

    doCompare();

}

void MainWindow::doCompare()
{

    m_diffs = QSharedPointer<QVector<byterange>>::create();
    //QVector<byterange> *temp = m_diffs.data();
    //dataSet::compare(*m_dataSet1.data(), dataSet2, *temp);
    dataSet::compare(*m_dataSet1.data(), dataSet2, *m_diffs.data());
   //  dataSet::compare(*m_dataSet1->getData(), dataSet2, *m_diffs.data());

    //subset = byterange(4,512);
    //byterange subset(0,1321);
    m_dataSetView1 = QSharedPointer<dataSetView>::create(m_dataSet1, m_diffs);
    m_dataSetView1->setSubset(byterange(4,128));

    //vectorToQTextEdit(ui->textEdit_dataSet1, dataSet1, diffs);
    //vectorSubsetToQTextEdit(ui->textEdit_dataSet1, dataSet1.data, subset, diffs);
    m_dataSetView1->vectorSubsetToQTextEdit(ui->textEdit_dataSet1);
   // vectorToQTextEdit(ui->textEdit_dataSet2, dataSet2.data, *m_diffs.data());
    ui->textEdit_dataSet2->clear();
    vectorToQTextEdit(ui->textEdit_dataSet2, *dataSet2.getData(), *m_diffs.data());
/*
    ui->L_DataSet1SizeBytes->setText(QStringLiteral("DataSet1 Size: %1 bytes").arg(m_dataSet1->getData()->size()));
    ui->L_DataSet2SizeBytes->setText(QStringLiteral("DataSet2 Size: %1 bytes").arg(dataSet2.getData()->size()));

    ui->L_DataSet1SizeOffset->setText(QStringLiteral("DataSet1 subset- start: %1  count: %2")
        .arg(m_dataSetView1->m_subset.start).arg(m_dataSetView1->m_subset.count));
*/
    connect(m_dataSetView1.data(), &dataSetView::subsetChanged, m_DebugWindow.data(), &DebugWindow::dataSet1RangeChanged);

    ui->verticalScrollBar->setMinimum(0);
    ui->verticalScrollBar->setMaximum(m_dataSet1->getData()->size() - m_dataSetView1->getSubset().count);
    //ui->verticalScrollBar->triggerAction(QAbstractSlider::SliderMove);
    //ui->verticalScrollBar->setValue(1);
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
        this->dataSet2.loadFile(filename);
    }
}
