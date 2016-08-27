#include "debugwindow.h"
#include "ui_debugwindow.h"

DebugWindow::DebugWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugWindow)
{
    ui->setupUi(this);
    ui->L_DataSet1SizeOffset->setText("");
    ui->L_DataSet2SizeOffset->setText("");
    ui->L_DataSet1SizeBytes->setText("");
    ui->L_DataSet2SizeBytes->setText("");
}

DebugWindow::~DebugWindow()
{
    delete ui;
}

void DebugWindow::dataSet1RangeChanged(byteRange newRange)
{
    ui->L_DataSet1SizeOffset->setText(QStringLiteral("DataSet1 subset- start: %1  count: %2")
        .arg(newRange.start).arg(newRange.count));
}

void DebugWindow::dataSet2RangeChanged(byteRange newRange)
{
    ui->L_DataSet2SizeOffset->setText(QStringLiteral("DataSet2 subset- start: %1  count: %2")
        .arg(newRange.start).arg(newRange.count));
}

void DebugWindow::dataSet1SizeChanged(unsigned int newSize)
{
    ui->L_DataSet1SizeBytes->setText(QStringLiteral("DataSet1 Size: %1 bytes").arg(newSize));
}

void DebugWindow::dataSet2SizeChanged(unsigned int newSize)
{
    ui->L_DataSet2SizeBytes->setText(QStringLiteral("DataSet2 Size: %1 bytes").arg(newSize));
}
