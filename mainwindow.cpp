#include "mainwindow.h"

/*static*/ const QString MainWindow::APPLICATION_TITLE = "Difference Finder v0.2";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_userSettings(),
    m_comparisonThread()
{
    ui->setupUi(this);

    refreshTitleBarText();

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
    connect(&m_comparisonThread, &comparisonThread::finished, this, &MainWindow::onComparisonThreadEnded);

    //set initial log area size (first entry should just be nonzero, 2nd one is initial log area size)
    ui->logAreaSplitter->setSizes(QList<int>() << 50 << 240);//120 );

    //make log area collapsible
    ui->logAreaSplitter->setCollapsible(1,true);


    //apply user settings (from defaults in m_userSettings)
    applyUserSettings();

    //set initial width for address column areas
    onHexFieldFontChange();

    //load settings from ini file
    m_userSettings.loadINIFile();

    //resize window from ini file settings
    if (m_userSettings.windowWidth && m_userSettings.windowHeight) {
        ASSERT_LE_INT_MAX(m_userSettings.windowWidth);
        ASSERT_LE_INT_MAX(m_userSettings.windowHeight);
        this->resize(   static_cast<int>(m_userSettings.windowWidth),
                        static_cast<int>(m_userSettings.windowHeight)   );
    }

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
    //record window size
    ASSERT(0 < this->width());
    ASSERT(0 < this->height());
    m_userSettings.windowWidth  = static_cast<unsigned int>(this->width() );
    m_userSettings.windowHeight = static_cast<unsigned int>(this->height());

    //save settings to ini file
    m_userSettings.saveINIFile();
    QMainWindow::closeEvent(event);
}

void MainWindow::doLoadFile1(const QString filename)
{
    dataSet::loadFileResult res = m_dataSet1->loadFile(filename);
    if      (res == dataSet::loadFileResult::SUCCESS) {
        //do nothing
    }
    else if (res == dataSet::loadFileResult::ERROR_FileDoesNotExist) {
        LOG.Error("File \"" % filename % "\" does not exist.");
    }
    else if (res == dataSet::loadFileResult::ERROR_FileReadFailure) {
        LOG.Error("\"" % filename % "\" failed to read.");
    }
    else if (res == dataSet::loadFileResult::ERROR_ActiveDataReadLock) {
        LOG.Error("File load canceled: Current file is still in use.");
    }
    else {
        FAIL();
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

        ui->label_File1Path->setText(m_dataSet1->getFileName());
    }

    refreshTitleBarText();
}

void MainWindow::doLoadFile2(const QString filename)
{
    dataSet::loadFileResult res = m_dataSet2->loadFile(filename);
    if      (res == dataSet::loadFileResult::SUCCESS) {
        //do nothing
    }
    else if (res == dataSet::loadFileResult::ERROR_FileDoesNotExist) {
        LOG.Error("File \"" % filename % "\" does not exist.");
    }
    else if (res == dataSet::loadFileResult::ERROR_FileReadFailure) {
        LOG.Error("\"" % filename % "\" failed to read.");
    }
    else if (res == dataSet::loadFileResult::ERROR_ActiveDataReadLock) {
        LOG.Error("File load canceled: Current file is still in use.");
    }
    else {
        FAIL();
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

        ui->label_File2Path->setText(m_dataSet2->getFileName());
    }

    refreshTitleBarText();
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

void MainWindow::refreshTitleBarText()
{
    if (    m_dataSet1 && m_dataSet1->isLoaded()
         && m_dataSet2 && m_dataSet2->isLoaded()) {

        QFileInfo fi1(m_dataSet1->getFileName());
        QFileInfo fi2(m_dataSet2->getFileName());

        //this uses QFileInfo objects to get filenames from the loaded file paths
        QString titleBarText = QString("%1 <-> %2 - %3").arg(fi1.fileName()).arg(fi2.fileName()).arg(APPLICATION_TITLE);
        setWindowTitle(titleBarText);

    } else {

        setWindowTitle(APPLICATION_TITLE);
    }
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

        const dataSet::DataReadLock& DRL = ds->getReadLock();

        ASSERT_LE_INT_MAX(DRL.getData().size());
        ASSERT_LE_INT_MAX(dsv->getSubset().count);
        int scrollRange = static_cast<int>(DRL.getData().size()) - static_cast<int>(dsv->getSubset().count);

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

void MainWindow::on_actionTest_Compare3_triggered()
{
    doLoadFile1("TestFiles/test1_1");
    doLoadFile2("TestFiles/test1_2");

    m_comparisonThread.setDataSet1(m_dataSet1);
    m_comparisonThread.setDataSet2(m_dataSet2);
    m_comparisonThread.doCompare();
}

void MainWindow::onComparisonThreadEnded()
{
    LOG.Debug("onComparisonThreadEnded");

    auto results = m_comparisonThread.getResults();

    if (!results) {
        LOG.Error("comparison results should be ready, but aren't");
        return;
    }

    if (results->aborted) {
        LOG.Info("comparison was aborted");
        return;
    }

    if (results->internalError) {
        LOG.Error("comparison failed due to an internal error");
        return;
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

void MainWindow::on_actionStop_thread_triggered()
{
    m_comparisonThread.abort();
}

void MainWindow::on_actionTest_triggered()
{
    STOPWATCH1.clear();
    STOPWATCH1.recordTime("test time:");


    const dataSet::DataReadLock& DRL1 = m_dataSet1->getReadLock();
    const std::vector<unsigned char> &dS1 = DRL1.getData();

    const dataSet::DataReadLock& DRL2 = m_dataSet2->getReadLock();
    const std::vector<unsigned char> &dS2 = DRL2.getData();

    std::list<byteRange> file1_matches;
    std::list<byteRange> file1_differences;
    std::list<byteRange> file2_matches;
    std::list<byteRange> file2_differences;

    unsigned int sourceStartIndex = 0;
    unsigned int targetStartIndex = 0;

    std::list<rangeMatch> alignmentRanges;

    while(1) {
        //auto rangeResult = offsetMetrics::getNextAlignmentRange(dS1, dS2, sourceStartIndex, targetStartIndex);


        std::list<byteRange> targetSearchRanges;
        {
            //find valid target search ranges (temp code, avoid copying list?)
            std::list<byteRange> alignmentRangesInTarget;

            for (rangeMatch& alignmentRange : alignmentRanges) {
                alignmentRangesInTarget.emplace_back(alignmentRange.startIndexInFile2, alignmentRange.byteCount);
            }

            alignmentRangesInTarget.sort();

            //fillEmptySpaces requires nondecreasing and nonoverlapping
            ASSERT(byteRange::                isNonOverlapping(alignmentRangesInTarget));
            ASSERT(byteRange::isNonDecreasingAndNonOverlapping(alignmentRangesInTarget));

            byteRange::fillEmptySpaces(byteRange(0, dS2.size()), alignmentRangesInTarget, targetSearchRanges);
        }

        std::unique_ptr<rangeMatch> rangeResult;
        if (DEBUGFLAG1)
        {
            rangeResult = offsetMetrics::getNextAlignmentRange(dS1, dS2,
                                                                    byteRange(sourceStartIndex, dS1.size() - sourceStartIndex),
                                                                    byteRange(targetStartIndex, dS2.size() - targetStartIndex));
        } else
        {
            rangeResult = offsetMetrics::getNextAlignmentRange(dS1, dS2,
                                                                    byteRange(sourceStartIndex, dS1.size() - sourceStartIndex),
                                                                    targetSearchRanges);
        }

        if (rangeResult){
            rangeMatch range = *rangeResult.release();
            LOG.Info(QString("getNextAlignmentRange: %1, %2; %3")
                            .arg(range.startIndexInFile1)
                            .arg(range.startIndexInFile2)
                            .arg(range.byteCount));

            sourceStartIndex = range.getEndInFile1();
            targetStartIndex = range.getEndInFile2();

            alignmentRanges.push_back(range);
        }
        else
        {
            LOG.Info(QString("getNextAlignmentRange returned nullptr"));
            break;
        }
    }

    for (const rangeMatch& alignmentRange : alignmentRanges) {

        offsetMetrics::getAlignmentRangeDiff(dS1, dS2, alignmentRange,
                                                        file1_matches,
                                                        file1_differences,
                                                        file2_matches,
                                                        file2_differences   );
    }


    if ( !m_dataSetView1 || !m_dataSetView2 ) {
        return;
    }

    m_dataSetView1->clearHighlighting();
    m_dataSetView2->clearHighlighting();

    //m_dataSetView1->addHighlighting(results->matches, true);
    //m_dataSetView2->addHighlighting(results->matches, false);
    //m_dataSetView1->addDiffHighlighting(alignmentRanges_file1);
    //m_dataSetView2->addDiffHighlighting(alignmentRanges_file2);
    m_dataSetView1->addHighlighting(file1_matches);
    m_dataSetView2->addHighlighting(file2_matches);
    m_dataSetView1->addDiffHighlighting(file1_differences);//matches);
    m_dataSetView2->addDiffHighlighting(file2_differences);//matches);

    m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
    m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);

    STOPWATCH1.recordTime();
    STOPWATCH1.reportTimes(&Log::strMessageLvl2);
}

void MainWindow::on_actionDebugFlag_triggered()
{
    DEBUGFLAG1 = !DEBUGFLAG1;
    LOG.Debug(QString("DEBUGFLAG1: %1").arg(DEBUGFLAG1));
}
