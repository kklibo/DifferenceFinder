#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_DebugWindow(new DebugWindow()),
    m_userSettings(),
    m_comparisonThread()
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

    connect(ui->textEdit_dataSet1, &hexField::resized, this, &MainWindow::resizeHexField1);
    connect(ui->textEdit_dataSet2, &hexField::resized, this, &MainWindow::resizeHexField2);

    connect(&LOG, &Log::message, this, &MainWindow::displayLogMessage);

    connect(&m_comparisonThread, &comparisonThread::sendMessage, this, &MainWindow::displayLogMessage);
    connect(&m_comparisonThread, &comparisonThread::resultsAreReady, this, &MainWindow::onComparisonThreadResultsReady);

    //set initial log area size (first entry should just be nonzero, 2nd one is initial log area size)
    ui->logAreaSplitter->setSizes(QList<int>() << 50 << 240);//120 );

    //make log area collapsible
    ui->logAreaSplitter->setCollapsible(1,true);


    //apply user settings (from defaults in m_userSettings)
    applyUserSettings();

    //set initial width for address column areas
    onHexFieldFontChange();

    LOG.Info("started");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::doScrollBar(int value)
{
    auto doScroll = [value](const QSharedPointer<dataSetView> dsv, QTextEdit* byteGrid, QTextEdit* addressColumn)
    {
        if (!dsv) {return;}

        ASSERT_NOT_NEGATIVE(value);
        unsigned int val = static_cast<unsigned int>(value);
        unsigned int bytesPerRow = dsv->getBytesPerRow();

        if (bytesPerRow) {
            if (dataSetView::ByteGridScrollingMode::FixedRows == dsv->byteGridScrollingMode) {
                //apply modulo for fixed row effect
                val -= val%bytesPerRow;
            }

            dsv->setSubsetStart(val);
            dsv->printByteGrid(byteGrid, addressColumn);
        }
    };

    doScroll(m_dataSetView1, ui->textEdit_dataSet1, ui->textEdit_address1);
    doScroll(m_dataSetView2, ui->textEdit_dataSet2, ui->textEdit_address2);
}

void MainWindow::on_actionShow_Debug_triggered()
{
    m_DebugWindow->show();
}

void MainWindow::resizeHexField1()
{
    if (m_dataSetView1) {
        m_dataSetView1->updateByteGridDimensions(ui->textEdit_dataSet1);
        m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
        updateScrollBarRange();
    }
}

void MainWindow::resizeHexField2()
{
    if (m_dataSetView2) {
        m_dataSetView2->updateByteGridDimensions(ui->textEdit_dataSet2);
        m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);
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

void MainWindow::on_actionTest_highlighting_triggered()
{
    doLoadFile1("test1");
    doLoadFile2("test2");

    doCompare();

    if (m_dataSetView1) {

        {
            auto ranges = QSharedPointer<QVector<byteRange>>::create();
            ranges.data()->append(byteRange(40,10));
            dataSetView::highlightSet hSet(ranges);
            hSet.setForegroundColor(QColor::fromRgb(128,0,0));
            hSet.setBackgroundColor(QColor::fromRgb(0,128,128));

            m_dataSetView1->addHighlightSet(hSet);
        }

        {
            auto ranges = QSharedPointer<QVector<byteRange>>::create();
            ranges.data()->append(byteRange(55,10));
            dataSetView::highlightSet hSet(ranges);
            hSet.setForegroundColor(QColor::fromRgb(128,255,0));
            hSet.setBackgroundColor(QColor::fromRgb(0,0,128));

            m_dataSetView1->addHighlightSet(hSet);
        }

        {
            auto ranges = QSharedPointer<QVector<byteRange>>::create();
            ranges.data()->append(byteRange(30,10));
            dataSetView::highlightSet hSet(ranges);
            hSet.setForegroundColor(QColor::fromRgb(255,255,255));
            hSet.setBackgroundColor(QColor::fromRgb(128,128,128));

            m_dataSetView1->addHighlightSet(hSet);
        }

        m_dataSetView1->updateByteGridDimensions(ui->textEdit_dataSet1);
        m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);

    }
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

    applyUserSettings(); //reapply user settings to new dataSetView

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

    applyUserSettings(); //reapply user settings to new dataSetView

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

void MainWindow::doSimpleCompare()
{
    if (m_dataSet1.isNull() || m_dataSet2.isNull()) {
        return;
    }

    auto m_diffs = QSharedPointer<QVector<byteRange>>::create();
    dataSet::compare(*m_dataSet1.data(), *m_dataSet2.data(), *m_diffs.data());

    //create highlightSet to color byte differences between files
    dataSetView::highlightSet hSet(m_diffs);
    hSet.setForegroundColor(QColor::fromRgb(0,128,0));
    hSet.setBackgroundColor(QColor::fromRgb(128,128,0));

    m_dataSetView1 = QSharedPointer<dataSetView>::create(m_dataSet1);
    applyUserSettings(); //reapply user settings to new dataSetView
    m_dataSetView1->addHighlightSet(hSet);

    m_dataSetView2 = QSharedPointer<dataSetView>::create(m_dataSet2);
    applyUserSettings(); //reapply user settings to new dataSetView
    m_dataSetView2->addHighlightSet(hSet);


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

void MainWindow::doCompare()
{
    LOG.Debug("MainWindow::doCompare");
    m_comparisonThread.setDataSet1(m_dataSet1);
    m_comparisonThread.setDataSet2(m_dataSet2);
    m_comparisonThread.doCompare();
}

void MainWindow::applyUserSettings()
{
    //applies user settings to a dataSetView
    auto apply = [this](QSharedPointer<dataSetView> ds){
        if (ds) {
            ds->byteGridColumnMode                  = m_userSettings.byteGridColumnMode;
            ds->byteGridColumn_LargestMultipleOf_N  = m_userSettings.byteGridColumn_LargestMultipleOf_N;
            ds->byteGridColumn_UpTo_N               = m_userSettings.byteGridColumn_UpTo_N;
            ds->byteGridScrollingMode               = m_userSettings.byteGridScrollingMode;
        }
    };

    apply(m_dataSetView1);
    apply(m_dataSetView2);
}

void MainWindow::updateScrollBarRange()
{
    ui->verticalScrollBar->setMinimum(0);

    //set scrollbar maximum value (being careful here, in case the two datasets or viewable subsets are different):
    //calculate the scroll range needed to completely show each dataset in its viewable subset

    auto getScrollRange = [](QSharedPointer<dataSet> ds, QSharedPointer<dataSetView> dsv)->int {
        if (!ds || !dsv) {
            return 0;   //default to 0 scroll range if not loaded
        }

        ASSERT_LE_INT_MAX(dsv->getSubset().count);
        int scrollRange = ds->getData()->size() - static_cast<int>(dsv->getSubset().count);

        if (dataSetView::ByteGridScrollingMode::FixedRows == dsv->byteGridScrollingMode) {
            ASSERT_LE_INT_MAX(dsv->getBytesPerRow());
            int bytesPerRow = static_cast<int>(dsv->getBytesPerRow());
            if (0 == bytesPerRow) {
                scrollRange = 0;
            }
            else if ( scrollRange%bytesPerRow ) {
                scrollRange += bytesPerRow;
            }
        }
        return scrollRange;
    };

    int scrollRange1 = getScrollRange(m_dataSet1, m_dataSetView1);
    int scrollRange2 = getScrollRange(m_dataSet2, m_dataSetView2);

    int scrollBarMax = qMax(scrollRange1, scrollRange2); //choose the max scroll range needed

    //ensure scrollbar maximum value is at least zero
    // to prevent bug when data size < view subset count (max could go negative)
    ui->verticalScrollBar->setMaximum(qMax(0, scrollBarMax));


    //set the scrollbar singleStep value to an entire row if both views are in FixedRows mode.
    //  this way, one click on the up or down button will scroll an entire row,
    //  instead of possibly having no visible effect until subsequent clicks add up to an entire row
    unsigned int scrollStep;
    if( m_dataSetView1 && m_dataSetView2
        && m_dataSetView1->byteGridScrollingMode == dataSetView::ByteGridScrollingMode::FixedRows
        && m_dataSetView2->byteGridScrollingMode == dataSetView::ByteGridScrollingMode::FixedRows )
    {
        //if the byte grids have different row sizes, pick the smaller one so click-scrolling won't skip data
        scrollStep = qMin(  m_dataSetView1->getBytesPerRow(),
                            m_dataSetView2->getBytesPerRow());
    } else {

        scrollStep = 1; //at least one view isn't in FixedRows mode, set scroll singleStep to 1 (byte)
    }
    ASSERT_LE_INT_MAX(scrollStep);
    ui->verticalScrollBar->setSingleStep(static_cast<int>(scrollStep));

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
STOPWATCH1.clear();
STOPWATCH1.recordTime("Compare operation Total:");
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

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog sd;
    sd.setModal(true);

    //initialize dialog with current settings
    sd.initUserSettings(m_userSettings);

    sd.exec();

    if (QDialog::Accepted == sd.result()) {
        //if OK was clicked, update current settings
        m_userSettings = sd.getUserSettings();

        applyUserSettings();

        //refresh data view settings and redraw
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

        //fire scrollbar handler (to update row alignment after a scroll settings change)
        doScrollBar(ui->verticalScrollBar->value());
    }
}

void MainWindow::on_actionTest_Compare1_triggered()
{
    comparison::rollingHashTest();
    comparison C;
    C.rollingHashTest2();
/*
    if (!m_dataSet1->getData() || !m_dataSet2->getData()) {
        return;
    }

    std::vector<unsigned char> dS1 = m_dataSet1->getData()->toStdVector();
    std::vector<unsigned char> dS2 = m_dataSet2->getData()->toStdVector();

    for (unsigned int i = 0; i <= qMin(dS1.size(), dS2.size()); ++i) {

        QString res;
        if (C.blockHashMatchExists(i, dS1, dS2)) {
            res = "yes";
        }
        else {
            res = "no";
        }
        LOG.Debug(QString("%1: %2").arg(i).arg(res));
    }

    return;*/

    //std::vector<unsigned char> test1 = {0,1,2,3,4,5,6,7,8,9};
    auto ranges = QSharedPointer<QVector<byteRange>>::create();

    auto test1 = [&C, &ranges](QSharedPointer<dataSet> dS){
        if(!dS){return;}

        std::vector<unsigned char> test1 = dS->getData()->toStdVector();

        for (int i = 0; i < 1; i++) {

            auto res = C.getRollingHashValues(test1);
            LOG.Debug(QString("rollingHashTest- data size: %1").arg(test1.size()));
            for (auto& v : *res.get()) {
                QString s = QString("0x%1").arg(v,8,16,QChar('0'));
                LOG.Debug(s);

               // ranges.data()->append(byteRange(v-4,1));

            }

        }
    };

    test1(m_dataSet1);
    test1(m_dataSet2);
/*
    if (m_dataSetView1) {

        {
            //auto ranges = QSharedPointer<QVector<byteRange>>::create();
            //ranges.data()->append(byteRange(40,10));
            dataSetView::highlightSet hSet(ranges);
            hSet.setForegroundColor(QColor::fromRgb(128,0,0));
            hSet.setBackgroundColor(QColor::fromRgb(0,128,128));

            m_dataSetView1->addHighlightSet(hSet);
        }



        m_dataSetView1->updateByteGridDimensions(ui->textEdit_dataSet1);
        m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);

    }
*/
}

void MainWindow::on_actionTest_Compare2_triggered()
{
    doLoadFile1("smalltest1");
    doLoadFile2("smalltest2");

    //comparison::rollingHashTest();
    comparison C;
    //C.rollingHashTest2();

    if (!m_dataSet1->getData() || !m_dataSet2->getData()) {
        return;
    }

    std::vector<unsigned char> dS1 = m_dataSet1->getData()->toStdVector();
    std::vector<unsigned char> dS2 = m_dataSet2->getData()->toStdVector();

    std::multiset<byteRange> data1SkipRanges;
    std::multiset<byteRange> data2SkipRanges;

    data1SkipRanges.emplace(2, 3);
    data1SkipRanges.emplace(6, 2);
    data2SkipRanges.emplace(3, 3);
    data2SkipRanges.emplace(7, 2);

    for (unsigned int i = 0; i <= qMin(dS1.size(), dS2.size()); ++i) {

        QString res;
        if (C.blockMatchSearch(i, dS1, dS2, data1SkipRanges, data2SkipRanges)) {
            res = "yes";
        }
        else {
            res = "no";
        }
        LOG.Debug(QString("%1: %2").arg(i).arg(res));
    }

    {
        std::multiset<blockMatchSet> matches;
        C.blockMatchSearch(3, dS1, dS2, data1SkipRanges, data2SkipRanges, &matches);
        LOG.Debug("!");
        m_dataSetView1->clearHighlighting();
        m_dataSetView2->clearHighlighting();
        m_dataSetView1->addHighlighting(matches, true);
        m_dataSetView2->addHighlighting(matches, false);
        m_dataSetView1->addHighlighting(data1SkipRanges);
        m_dataSetView2->addHighlighting(data2SkipRanges);
        m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
        m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);
    }


}

void MainWindow::on_actionTest_Compare3_triggered()
{
    doLoadFile1("test1");
    doLoadFile2("test2");

    m_comparisonThread.setDataSet1(m_dataSet1);
    m_comparisonThread.setDataSet2(m_dataSet2);
    m_comparisonThread.doCompare();
}

void MainWindow::on_actionUnit_tester_triggered()
{
    byteRange::test();
}

void MainWindow::on_actionThread_test1_triggered()
{
    LOG.Debug("on_actionThread_test1_triggered");
    m_comparisonThread.setDataSet1(m_dataSet1);
    m_comparisonThread.setDataSet2(m_dataSet2);
    m_comparisonThread.doCompare();
}

void MainWindow::onComparisonThreadResultsReady()
{
    LOG.Debug("onComparisonThreadResultsReady");

    auto results = m_comparisonThread.getResults();
    if (!results) {
        LOG.Error("comparison results should be ready, but aren't");
    }

    if ( !m_dataSetView1 || !m_dataSetView2 ) {
        return;
    }

    m_dataSetView1->clearHighlighting();
    m_dataSetView2->clearHighlighting();

    m_dataSetView1->addHighlighting(results->matches, true);
    m_dataSetView2->addHighlighting(results->matches, false);
    m_dataSetView1->addDiffHighlighting(results->data1_unmatchedBlocks);
    m_dataSetView2->addDiffHighlighting(results->data2_unmatchedBlocks);

    //m_dataSetView1->addHighlighting(data1SkipRanges);
    //m_dataSetView2->addHighlighting(data2SkipRanges);
    m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
    m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);

STOPWATCH1.recordTime();
STOPWATCH1.reportTimes(&Log::strMessageLvl2);
}

void MainWindow::on_actionDoSimpleCompare_triggered()
{
    doSimpleCompare();
}
