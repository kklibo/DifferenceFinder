#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_DebugWindow(new DebugWindow())
{
    ui->setupUi(this);

    //create dataSets
    m_dataSet1 = QSharedPointer<dataSet>::create();
    m_dataSet2 = QSharedPointer<dataSet>::create();

    connect(ui->verticalScrollBar, &QScrollBar::valueChanged, this, &MainWindow::doScrollBar);

    connect(ui->textEdit_dataSet1, &hexField::filenameDropped, this, &MainWindow::doLoadFile1);
    connect(ui->textEdit_dataSet2, &hexField::filenameDropped, this, &MainWindow::doLoadFile2);

    connect(ui->textEdit_dataSet1, &hexField::fontChanged, this, &MainWindow::onHexFieldFontChange);
    connect(ui->textEdit_dataSet2, &hexField::fontChanged, this, &MainWindow::onHexFieldFontChange);

    connect(&LOG, &Log::message, this, &MainWindow::displayLogMessage);

    //set initial log area size (first entry should just be nonzero, 2nd one is initial log area size)
    ui->logAreaSplitter->setSizes(QList<int>() << 50 << 120 );

    //make log area collapsible
    ui->logAreaSplitter->setCollapsible(1,true);

    //set text color for data view areas
    ui->textEdit_dataSet1->setTextColor(QColor::fromRgb(0,0,0));
    ui->textEdit_dataSet2->setTextColor(QColor::fromRgb(0,0,0));

    //set width & text color for address column areas
    onHexFieldFontChange();

    LOG.Info("started");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::doScrollBar(int value)
{
    if (m_dataSetView1) {
        m_dataSetView1->setSubsetStart(value);
        m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
    }

    if (m_dataSetView2) {
        m_dataSetView2->setSubsetStart(value);
        m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);
    }
}

void MainWindow::on_actionShow_Debug_triggered()
{
    m_DebugWindow->show();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if (m_dataSetView1) {
        m_dataSetView1->updateByteGridDimensions(ui->textEdit_dataSet1);
        m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
    }

    if (m_dataSetView2) {
        m_dataSetView2->updateByteGridDimensions(ui->textEdit_dataSet2);
        m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);
    }

    if (m_dataSetView1 || m_dataSetView2) {
        updateScrollBarRange();
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    m_DebugWindow->close();
    QMainWindow::closeEvent(event);
}

void MainWindow::on_actionTest_triggered()
{
    doLoadFile1("test1");
    doLoadFile2("test2");

    doCompare();
}

void MainWindow::doLoadFile1(const QString filename)
{
    dataSet::loadFileResult res = m_dataSet1->loadFile(filename);
    if      (res == dataSet::loadFileResult::ERROR_FileDoesNotExist) {
        LOG.Error("File \"" % filename % "\" does not exist.");
    }
    else if (res == dataSet::loadFileResult::ERROR_FileReadFailure) {
        LOG.Error("\"" % filename % "\" failed to read.");
    }

    m_dataSetView1 = QSharedPointer<dataSetView>::create(m_dataSet1);

    if (m_dataSet1->isLoaded() && m_dataSetView1) {

        //synchronize displayed data range start indices
        if (m_dataSetView2) {
            m_dataSetView1->setSubsetStart(m_dataSetView2->getSubsetStart());
        }

        m_dataSetView1->updateByteGridDimensions(ui->textEdit_dataSet1);
        m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
        updateScrollBarRange();
    }
}

void MainWindow::doLoadFile2(const QString filename)
{
    dataSet::loadFileResult res = m_dataSet2->loadFile(filename);
    if      (res == dataSet::loadFileResult::ERROR_FileDoesNotExist) {
        LOG.Error("File \"" % filename % "\" does not exist.");
    }
    else if (res == dataSet::loadFileResult::ERROR_FileReadFailure) {
        LOG.Error("\"" % filename % "\" failed to read.");
    }

    m_dataSetView2 = QSharedPointer<dataSetView>::create(m_dataSet2);

    if (m_dataSet2->isLoaded() && m_dataSetView2) {

        //synchronize displayed data range start indices
        if (m_dataSetView1) {
            m_dataSetView2->setSubsetStart(m_dataSetView1->getSubsetStart());
        }

        m_dataSetView2->updateByteGridDimensions(ui->textEdit_dataSet2);
        m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);
        updateScrollBarRange();
    }
}

void MainWindow::doCompare()
{
    if (m_dataSet1.isNull() || m_dataSet2.isNull()) {
        return;
    }

    m_diffs = QSharedPointer<QVector<byteRange>>::create();
    dataSet::compare(*m_dataSet1.data(), *m_dataSet2.data(), *m_diffs.data());

    m_dataSetView1 = QSharedPointer<dataSetView>::create(m_dataSet1, m_diffs);
    m_dataSetView2 = QSharedPointer<dataSetView>::create(m_dataSet2, m_diffs);

    if (m_dataSetView1.isNull() || m_dataSetView2.isNull()) {
        return;
    }

    m_dataSetView1->updateByteGridDimensions(ui->textEdit_dataSet1);
    m_dataSetView2->updateByteGridDimensions(ui->textEdit_dataSet2);

    m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
    m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);

    updateScrollBarRange();


    connect(m_dataSetView1.data(), &dataSetView::subsetChanged, m_DebugWindow.data(), &DebugWindow::dataSet1RangeChanged);
    connect(m_dataSetView2.data(), &dataSetView::subsetChanged, m_DebugWindow.data(), &DebugWindow::dataSet2RangeChanged);
}

void MainWindow::updateScrollBarRange()
{
    ui->verticalScrollBar->setMinimum(0);

    //set scrollbar maximum value (being careful here, in case the two datasets or viewable subsets are different):
    //calculate the scroll range needed to completely show each dataset in its viewable subset

    int scrollRange1, scrollRange2 = 0; //default to 0 if not loaded

    if (m_dataSet1 && m_dataSetView1) {
        scrollRange1 = m_dataSet1->getData()->size() - m_dataSetView1->getSubset().count;
    }

    if (m_dataSet2 && m_dataSetView2) {
        scrollRange2 = m_dataSet2->getData()->size() - m_dataSetView2->getSubset().count;
    }

    int scrollBarMax = qMax(scrollRange1, scrollRange2); //choose the max scroll range needed

    //ensure scrollbar maximum value is at least zero
    // to prevent bug when data size < view subset count (max could go negative)
    ui->verticalScrollBar->setMaximum(qMax(0, scrollBarMax));
}

void MainWindow::displayLogMessage(QString str, QColor color)
{
    QColor orig = ui->textEdit_log->textColor();
    ui->textEdit_log->setTextColor(color);

    ui->textEdit_log->append(str);

    ui->textEdit_log->setTextColor(orig);
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
    if (!filename.isEmpty()){
        doLoadFile1(filename);
    }
}

void MainWindow::on_actionLoad_File2_Right_triggered()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, "Load File 2 (Left)");
    if (!filename.isEmpty()){
        doLoadFile2(filename);
    }
}

void MainWindow::onHexFieldFontChange()
{
    //prevent misaligned/hidden address text:
    //  make sure address column areas use the same font as their matching hexfield
    ui->textEdit_address1->setFont(ui->textEdit_dataSet1->font());
    ui->textEdit_address2->setFont(ui->textEdit_dataSet2->font());
    //todo: handle font changes from address column?


    auto setAddressColumnWidth = [](QTextEdit* const textEdit)->void
    {
        //calculate the QTextEdit width needed to draw the address text
        //
        //  The drawable area is textEdit->viewport().
        //  Text starts a bit inside this area; we can see where by moving the cursor
        //  to the start of the control and measuring the cursor location.
        textEdit->moveCursor(QTextCursor::Start);
        QPoint topLeftCornerOfFirstChar = textEdit->cursorRect().topLeft();

        //margins inside the viewport not used for text drawing
        int leftMargin  = textEdit->contentsMargins().left();
        int rightMargin = textEdit->contentsMargins().right();

        QFontMetrics qfm(textEdit->font());
        int addressWidth_px = qfm.width("0x00000000")   //get the width of an address (assuming MONOSPACE FONTS)
                    + topLeftCornerOfFirstChar.x()      //plus the distance from the left edge to text start
                    + leftMargin + rightMargin;         //plus the margins

        //set the min & max width values; qt should always draw it at this width
        textEdit->setFixedWidth(addressWidth_px);

    };

    setAddressColumnWidth(ui->textEdit_address1);
    setAddressColumnWidth(ui->textEdit_address2);

    ui->textEdit_dataSet1->setTextColor(QColor::fromRgb(64,64,128));
    ui->textEdit_dataSet2->setTextColor(QColor::fromRgb(64,64,128));



    if (m_dataSetView1) {
        m_dataSetView1->updateByteGridDimensions(ui->textEdit_dataSet1);
        m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
    }

    if (m_dataSetView2) {
        m_dataSetView2->updateByteGridDimensions(ui->textEdit_dataSet2);
        m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);
    }

    if (m_dataSetView1 || m_dataSetView2) {
        updateScrollBarRange();
    }

}
