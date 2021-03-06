#include "mainwindow.h"

scrollWheelRedirector::scrollWheelRedirector(QObject *redirectTo)
{
    m_redirectTo = redirectTo;
}

bool scrollWheelRedirector::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_redirectTo) {
        FAIL();
        return false;
    }

    if (event->type() == QEvent::Wheel) {
        //redirect QWheelEvents to the designated QObject
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);

        QCoreApplication::sendEvent(m_redirectTo, wheelEvent);
        return true;
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}


scrollBarKeyFilter::scrollBarKeyFilter(QScrollBar *scrollBar)
{
    m_QScrollBar = scrollBar;
}

bool scrollBarKeyFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_QScrollBar) {
        FAIL();
        return false;
    }

    if (event->type() == QEvent::KeyPress) {
        //use these keypresses to control the scrollbar
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        //down/up key multiplier
        int count = 1;

        if (keyEvent->modifiers() & Qt::ShiftModifier) {
            count = 4;
        }
        else if (keyEvent->modifiers() & Qt::ControlModifier) {
            count = 8;
        }

        switch (keyEvent->key()) {

        case Qt::Key_PageDown:
            m_QScrollBar->triggerAction(QAbstractSlider::SliderPageStepAdd);
            return true;

        case Qt::Key_PageUp:
            m_QScrollBar->triggerAction(QAbstractSlider::SliderPageStepSub);
            return true;

        case Qt::Key_Down:
            for (int i = 0; i < count; ++i) {
                m_QScrollBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
            }
            return true;

        case Qt::Key_Up:
            for (int i = 0; i < count; ++i) {
                m_QScrollBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
            }
            return true;

        case Qt::Key_End:
            m_QScrollBar->triggerAction(QAbstractSlider::SliderToMaximum);
            return true;

        case Qt::Key_Home:
            m_QScrollBar->triggerAction(QAbstractSlider::SliderToMinimum);
            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}


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

    //redirect scroll wheel events from the hex views to the main scrollbar
    m_scrollWheelRedirector = QSharedPointer<scrollWheelRedirector>::create(ui->verticalScrollBar);
    ui->textEdit_dataSet1->installEventFilter(m_scrollWheelRedirector.data());
    ui->textEdit_dataSet2->installEventFilter(m_scrollWheelRedirector.data());

    //use application-wide key event filter for scrollbar key controls
    m_scrollBarKeyFilter = QSharedPointer<scrollBarKeyFilter>::create(ui->verticalScrollBar);
    QApplication::instance()->installEventFilter(m_scrollBarKeyFilter.data());

    //load settings from ini file
    m_userSettings.loadINIFile();

    //resize window from ini file settings
    if (m_userSettings.windowWidth && m_userSettings.windowHeight) {
        ASSERT_LE_INT_MAX(m_userSettings.windowWidth);
        ASSERT_LE_INT_MAX(m_userSettings.windowHeight);
        this->resize(   static_cast<int>(m_userSettings.windowWidth),
                        static_cast<int>(m_userSettings.windowHeight)   );
    }

    //make log area collapsible
    ui->logAreaSplitter->setCollapsible(1,true);

    //set initial log area size (first entry should just be nonzero, 2nd one is initial log area size)
    ASSERT_LE_INT_MAX(m_userSettings.logAreaHeight);
    ui->logAreaSplitter->setSizes(QList<int>() << 50 << static_cast<int>(m_userSettings.logAreaHeight));

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

    //record log area height
    ASSERT_NOT_NEGATIVE(ui->textEdit_log->height());
    m_userSettings.logAreaHeight = static_cast<unsigned int>(ui->textEdit_log->height());

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

    updateUIforFile1Load();
}

void MainWindow::doLoadFile1FromMemory(std::unique_ptr<std::vector<unsigned char>> data)
{
    dataSet::loadFromMemoryResult res = m_dataSet1->loadFromMemory(std::move(data));
    if      (res == dataSet::loadFromMemoryResult::SUCCESS) {
        //do nothing
    }
    else if (res == dataSet::loadFromMemoryResult::ERROR_ActiveDataReadLock) {
        LOG.Error("File load canceled: Current file is still in use.");
    }
    else {
        FAIL();
    }

    updateUIforFile1Load();
}

void MainWindow::updateUIforFile1Load()
{
    m_dataSetView1 = QSharedPointer<dataSetView>::create(m_dataSet1);

    applyUserSettingsTo(m_dataSetView1); //reapply user settings to new dataSetView

    if (m_dataSet1->isLoaded() && m_dataSetView1) {

        //synchronize displayed data range start indices
        if (m_dataSetView2) {
            m_dataSetView1->setSubsetStart(m_dataSetView2->getSubsetStart());
        }

        m_dataSetView1->updateByteGridDimensions(ui->textEdit_dataSet1);
        m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
        updateScrollBarRange();

        QString name;
        dataSet::sourceInfo sourceInfo = m_dataSet1->getSourceInfo();

        if      (sourceInfo.type == dataSet::sourceType::file) {
            name = sourceInfo.name;
        }
        else if (sourceInfo.type == dataSet::sourceType::memory) {
            name = "[loaded from memory]";
        }
        else if (sourceInfo.type == dataSet::sourceType::none) {
            name = "[not loaded]";
        }
        else {
            FAIL();
        }

        QString filePathText = QString("[ %1 bytes ]     %2")
                                .arg(m_dataSet1->getSize())
                                .arg(name);

        ui->label_File1Path->setText(filePathText);
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

    updateUIforFile2Load();
}

void MainWindow::doLoadFile2FromMemory(std::unique_ptr<std::vector<unsigned char>> data)
{
    dataSet::loadFromMemoryResult res = m_dataSet2->loadFromMemory(std::move(data));
    if      (res == dataSet::loadFromMemoryResult::SUCCESS) {
        //do nothing
    }
    else if (res == dataSet::loadFromMemoryResult::ERROR_ActiveDataReadLock) {
        LOG.Error("File load canceled: Current file is still in use.");
    }
    else {
        FAIL();
    }

    updateUIforFile2Load();
}

void MainWindow::updateUIforFile2Load()
{
    m_dataSetView2 = QSharedPointer<dataSetView>::create(m_dataSet2);

    applyUserSettingsTo(m_dataSetView2); //reapply user settings to new dataSetView

    if (m_dataSet2->isLoaded() && m_dataSetView2) {

        //synchronize displayed data range start indices
        if (m_dataSetView1) {
            m_dataSetView2->setSubsetStart(m_dataSetView1->getSubsetStart());
        }

        m_dataSetView2->updateByteGridDimensions(ui->textEdit_dataSet2);
        m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);
        updateScrollBarRange();

        QString name;
        dataSet::sourceInfo sourceInfo = m_dataSet2->getSourceInfo();

        if      (sourceInfo.type == dataSet::sourceType::file) {
            name = sourceInfo.name;
        }
        else if (sourceInfo.type == dataSet::sourceType::memory) {
            name = "[loaded from memory]";
        }
        else if (sourceInfo.type == dataSet::sourceType::none) {
            name = "[not loaded]";
        }
        else {
            FAIL();
        }

        QString filePathText = QString("[ %1 bytes ]     %2")
                                .arg(m_dataSet2->getSize())
                                .arg(name);

        ui->label_File2Path->setText(filePathText);
    }

    refreshTitleBarText();
}

void MainWindow::doSimpleCompare()
{
    if (m_dataSet1.isNull() || m_dataSet2.isNull()) {
        return;
    }

    auto m_diffs = QSharedPointer<QVector<indexRange>>::create();
    dataSet::compare(*m_dataSet1.data(), *m_dataSet2.data(), *m_diffs.data());

    //create highlightSet to color byte differences between files
    dataSetView::highlightSet hSet(m_diffs);
    hSet.setForegroundColor(QColor::fromRgb(0,128,0));
    hSet.setBackgroundColor(QColor::fromRgb(128,128,0));

    m_dataSetView1 = QSharedPointer<dataSetView>::create(m_dataSet1);
    applyUserSettingsTo(m_dataSetView1); //reapply user settings to new dataSetView
    m_dataSetView1->addHighlightSet(hSet);

    m_dataSetView2 = QSharedPointer<dataSetView>::create(m_dataSet2);
    applyUserSettingsTo(m_dataSetView2); //reapply user settings to new dataSetView
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

void MainWindow::applyUserSettingsTo(QSharedPointer<dataSetView> ds)
{
    //applies user settings to a dataSetView
    if (ds) {
        ds->byteGridColumnMode                  = m_userSettings.byteGridColumnMode;
        ds->byteGridColumn_LargestMultipleOf_N  = m_userSettings.byteGridColumn_LargestMultipleOf_N;
        ds->byteGridColumn_UpTo_N               = m_userSettings.byteGridColumn_UpTo_N;
        ds->byteGridScrollingMode               = m_userSettings.byteGridScrollingMode;
    }
}

void MainWindow::refreshTitleBarText()
{
    if (    m_dataSet1 && m_dataSet1->isLoaded()
         && m_dataSet2 && m_dataSet2->isLoaded()
         && m_dataSet1->getSourceInfo().type == dataSet::sourceType::file
         && m_dataSet2->getSourceInfo().type == dataSet::sourceType::file ) {

        QFileInfo fi1(m_dataSet1->getSourceInfo().name);
        QFileInfo fi2(m_dataSet2->getSourceInfo().name);

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
        ASSERT_LE_INT_MAX(dsv->getSubset().count());
        int scrollRange = static_cast<int>(DRL.getData().size()) - static_cast<int>(dsv->getSubset().count());

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


    //set the scrollbar pageStep value to an entire visible page (rows x columns)
    unsigned int scrollPage;
    if( m_dataSetView1 && m_dataSetView2 )
    {
        //if the byte grids have different page sizes, pick the smaller one so page stepping won't skip data
        scrollPage = qMin(  m_dataSetView1->getSubset().count(),
                            m_dataSetView2->getSubset().count());

        ASSERT_LE_INT_MAX(scrollPage);
        ui->verticalScrollBar->setPageStep(static_cast<int>(scrollPage));
    }
}

void MainWindow::displayLogMessage(QString str, QColor color)
{
    QColor orig = ui->textEdit_log->textColor();
    ui->textEdit_log->setTextColor(color);

    ui->textEdit_log->append(str);

    ui->textEdit_log->setTextColor(orig);
}

QString MainWindow::summarizeResults(const comparison::results& results)
{
    if (results.aborted) {
        return "aborted";
    }
    if (results.internalError) {
        return "internal error";
    }

    unsigned int matchedBlocksInData1 = 0;
    unsigned int matchedBlocksInData2 = 0;
    unsigned int matchedBytesInData1 = 0;
    unsigned int matchedBytesInData2 = 0;


    for (const blockMatchSet& bms : results.matches) {

        matchedBlocksInData1 += bms.data1_BlockStartIndices.size();
        matchedBlocksInData2 += bms.data2_BlockStartIndices.size();

        matchedBytesInData1 += bms.data1_BlockStartIndices.size() * bms.blockSize;
        matchedBytesInData2 += bms.data2_BlockStartIndices.size() * bms.blockSize;
    }

    auto getRangeTotal = [](const std::list<indexRange>& indexRanges) -> unsigned int {

        unsigned int totalBytes = 0;
        for (const indexRange& b : indexRanges) {
            totalBytes += b.count();
        }
        return totalBytes;
    };

    QString ret = QString(  "Largest Block Comparison Results:\n"
                            "matches.size(): %1\n"
                            "matched Blocks in Data1: %2\n"
                            "matched Blocks in Data2: %3\n"
                            "matched Bytes in Data1: %4\n"
                            "matched Bytes in Data2: %5\n"
                            "data1_unmatchedBlocks.size(): %6\n"
                            "data2_unmatchedBlocks.size(): %7\n"
                            "unmatched Bytes in Data1: %8\n"
                            "unmatched Bytes in Data2: %9\n"
                         )
                            .arg(results.matches.size())
                            .arg(matchedBlocksInData1)
                            .arg(matchedBlocksInData2)
                            .arg(matchedBytesInData1)
                            .arg(matchedBytesInData2)
                            .arg(results.data1_unmatchedBlocks.size())
                            .arg(results.data2_unmatchedBlocks.size())
                            .arg(getRangeTotal(results.data1_unmatchedBlocks))
                            .arg(getRangeTotal(results.data2_unmatchedBlocks))
                         ;

    return ret;
}

QString MainWindow::summarizeResults(const offsetMetrics::results& results)
{
    if (results.aborted) {
        return "aborted";
    }
    if (results.internalError) {
        return "internal error";
    }

    auto getRangeTotal = [](const std::list<indexRange>& indexRanges) -> unsigned int {

        unsigned int totalBytes = 0;
        for (const indexRange& b : indexRanges) {
            totalBytes += b.count();
        }
        return totalBytes;
    };


    QString ret = QString(  "Sequential Comparison Results:\n"
                            "file1_matches.size(): %1\n"
                            "file2_matches.size(): %2\n"
                            "file1_differences.size(): %3\n"
                            "file2_differences.size(): %4\n"
                            "total file1 matched bytes:   %5\n"
                            "total file2 matched bytes:   %6\n"
                            "total file1 different bytes: %7\n"
                            "total file2 different bytes: %8\n"
                         )
                            .arg(results.file1_matches.size())
                            .arg(results.file2_matches.size())
                            .arg(results.file1_differences.size())
                            .arg(results.file2_differences.size())
                            .arg(getRangeTotal(results.file1_matches))
                            .arg(getRangeTotal(results.file2_matches))
                            .arg(getRangeTotal(results.file1_differences))
                            .arg(getRangeTotal(results.file2_differences))
                         ;
    return ret;
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

        //refresh data view settings and redraw
        if (m_dataSetView1) {
            applyUserSettingsTo(m_dataSetView1);
            m_dataSetView1->updateByteGridDimensions(ui->textEdit_dataSet1);
            m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
        }

        if (m_dataSetView2) {
            applyUserSettingsTo(m_dataSetView2);
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

    on_actionSequential_compare_triggered();
}

void MainWindow::onComparisonThreadEnded()
{
    LOG.Debug("onComparisonThreadEnded");


    auto results_largestBlock = m_comparisonThread.getResults_largestBlock();
    auto results_sequential   = m_comparisonThread.getResults_sequential();

    if (results_largestBlock) {

        if (results_largestBlock->aborted) {
            LOG.Info("comparison was aborted");
            return;
        }

        if (results_largestBlock->internalError) {
            LOG.Error("comparison failed due to an internal error");
            return;
        }

        if ( !m_dataSetView1 || !m_dataSetView2 ) {
            return;
        }
        m_dataSetView1->clearHighlighting();
        m_dataSetView2->clearHighlighting();

        auto& results = *results_largestBlock;
        m_dataSetView1->addHighlighting(results.matches, true);
        m_dataSetView2->addHighlighting(results.matches, false);
        m_dataSetView1->addDiffHighlighting(results.data1_unmatchedBlocks);
        m_dataSetView2->addDiffHighlighting(results.data2_unmatchedBlocks);

        LOG.Info(summarizeResults(results));

        if (m_dataSet1) {
            LOG.Info(QString("File 1 Size: %1 bytes").arg(m_dataSet1->getSize()));
        }

        if (m_dataSet2) {
            LOG.Info(QString("File 2 Size: %1 bytes").arg(m_dataSet2->getSize()));
        }
    }
    else if (results_sequential) {

        if (results_sequential->aborted) {
            LOG.Info("comparison was aborted");
            return;
        }

        if (results_sequential->internalError) {
            LOG.Error("comparison failed due to an internal error");
            return;
        }

        if ( !m_dataSetView1 || !m_dataSetView2 ) {
            return;
        }
        m_dataSetView1->clearHighlighting();
        m_dataSetView2->clearHighlighting();

        auto& results = *results_sequential;
        m_dataSetView1->addHighlighting(results.file1_matches);
        m_dataSetView2->addHighlighting(results.file2_matches);
        m_dataSetView1->addDiffHighlighting(results.file1_differences);
        m_dataSetView2->addDiffHighlighting(results.file2_differences);

        LOG.Info(summarizeResults(results));

        if (m_dataSet1) {
            LOG.Info(QString("File 1 Size: %1 bytes").arg(m_dataSet1->getSize()));
        }

        if (m_dataSet2) {
            LOG.Info(QString("File 2 Size: %1 bytes").arg(m_dataSet2->getSize()));
        }
    }
    else {
        LOG.Error("comparison results should be ready, but aren't");
        return;
    }


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
    if (!m_dataSet1) {
        return;
    }

    std::unique_ptr<std::vector<unsigned char>> offsetMap;

    {
        const dataSet::DataReadLock& DRL1 = m_dataSet1->getReadLock();
        const dataSet::DataReadLock& DRL2 = m_dataSet2->getReadLock();

        //unsigned int temp =
        //        utilities::findStrongestRepetitionPeriod(DRL1.getData(), byteRange(0,DRL1.getData().size()));

        //LOG.Debug(QString("dataSet 1 findStrongestRepetitionPeriod: %1").arg(temp));

        //offsetMap = utilities::createOffsetByteMap(DRL1.getData(), byteRange(0,DRL1.getData().size()));

        offsetMap = utilities::createCrossFileOffsetByteMap(DRL1.getData(), indexRange(0,DRL1.getData().size()),
                                                            DRL2.getData(), indexRange(0,DRL2.getData().size()),
                                                            DEBUGFLAG1 );
    }

    doLoadFile1FromMemory(std::move(offsetMap));

    m_dataSetView1->clearHighlighting();
    m_dataSetView1->addByteColorHighlighting();
    m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);

/*
    doLoadFile2FromMemory(std::move(offsetMap));

    m_dataSetView2->clearHighlighting();
    m_dataSetView2->addByteColorHighlighting();
    m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);
*/

}

void MainWindow::on_actionDebugFlag_triggered()
{
    DEBUGFLAG1 = !DEBUGFLAG1;
    LOG.Debug(QString("DEBUGFLAG1: %1").arg(DEBUGFLAG1));
}

void MainWindow::on_actionSequential_compare_triggered()
{
STOPWATCH1.clear();
STOPWATCH1.recordTime("Sequential Compare operation Total:");

    if (m_comparisonThread.isRunning()) {
        LOG.Debug("sequential: m_comparisonThread already running");
        return;
    }

    m_comparisonThread.setDataSet1(m_dataSet1);
    m_comparisonThread.setDataSet2(m_dataSet2);
    m_comparisonThread.startThread(comparisonThread::comparisonAlgorithm::sequential)
        ?   LOG.Debug("starting Sequential Comparison")
        :   LOG.Debug("failed to start Sequential Comparison: worker thread already running");
}

void MainWindow::on_actionLargestBlock_compare_triggered()
{
STOPWATCH1.clear();
STOPWATCH1.recordTime("Largest Block Compare operation Total:");

    if (m_comparisonThread.isRunning()) {
        LOG.Debug("largest block: m_comparisonThread already running");
        return;
    }

    m_comparisonThread.setDataSet1(m_dataSet1);
    m_comparisonThread.setDataSet2(m_dataSet2);
    m_comparisonThread.startThread(comparisonThread::comparisonAlgorithm::largestBlock)
        ?   LOG.Debug("starting Largest Block Comparison")
        :   LOG.Debug("failed to start Largest Block Comparison: worker thread already running");
}

void MainWindow::on_actionSwitch_files_triggered()
{
    if (    m_dataSet1 && m_dataSet1->isLoaded()
         && m_dataSet2 && m_dataSet2->isLoaded() ) {

        dataSet::sourceInfo sourceInfo1 = m_dataSet1->getSourceInfo();
        dataSet::sourceInfo sourceInfo2 = m_dataSet2->getSourceInfo();

        if (    sourceInfo1.type == dataSet::sourceType::file
             && sourceInfo2.type == dataSet::sourceType::file ) {

            doLoadFile1(sourceInfo2.name);
            doLoadFile2(sourceInfo1.name);
        }
    }
}

void MainWindow::on_actionTest_load_triggered()
{
    doLoadFile1("TestFiles/random1k");
    doLoadFile2("TestFiles/random1k_removeFirst8of2ndHalf");
}


void MainWindow::doSearchProcessing(searchProcessing::searchAction action)
{
    //todo: eventually move this to searchProcessing class?


    //todo: fix parentState
    static std::shared_ptr<searchProcessing::searchState> parentState = nullptr;
   // static std::unique_ptr<searchProcessing::searchState> nextState   = nullptr;
    static std::shared_ptr<searchProcessing::searchState> nextState   = nullptr;

    static std::list<indexRange> file1_matches;
    static std::list<indexRange> file1_differences;
    static std::list<indexRange> file2_matches;
    static std::list<indexRange> file2_differences;

    if ( !m_dataSet1 || !m_dataSet2 ) {
        return;
    }

    const dataSet::DataReadLock& DRL1 = m_dataSet1->getReadLock();
    const dataSet::DataReadLock& DRL2 = m_dataSet2->getReadLock();

    static indexRange dataSet1Range;
    static indexRange dataSet2Range;

    dataSet1Range.end = DRL1.getData().size();
    dataSet2Range.end = DRL2.getData().size();


    if (searchProcessing::searchAction::none == action) {
        //reset
        parentState = nullptr;
        nextState   = nullptr;
        file1_matches.clear();
        file1_differences.clear();
        file2_matches.clear();
        file2_differences.clear();
        dataSet1Range.start = 0;
        dataSet2Range.start = 0;

        //clear
        if (m_dataSetView1 && m_dataSetView2) {

            m_dataSetView1->clearHighlighting();
            m_dataSetView2->clearHighlighting();

            m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
            m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);
        }
        return;
    }


    if (nextState) {

        dataSet1Range = nextState->dataSet1NextRange;
        dataSet2Range = nextState->dataSet2NextRange;

    }

    nextState = searchProcessing::doSearch(DRL1.getData(), DRL2.getData(),
                                            //parentState,
                                            nextState,
                                            action,
                                            dataSet1Range, dataSet2Range
                                            );

    if (nextState && nextState->newAlignmentRange) {

        ASSERT( nextState->newAlignmentRange->inDataSet1.count() == nextState->newAlignmentRange->inDataSet2.count() );
        rangeMatch RM(nextState->newAlignmentRange->inDataSet1.start,
                      nextState->newAlignmentRange->inDataSet2.start,
                      nextState->newAlignmentRange->inDataSet1.count());

        //add new match and diff ranges from new alignment range
        offsetMetrics::getAlignmentRangeDiff(DRL1.getData(), DRL2.getData(),
                                             RM,
                                             file1_matches,
                                             file1_differences,
                                             file2_matches,
                                             file2_differences);

        //display
        if (m_dataSetView1 && m_dataSetView2) {

            m_dataSetView1->clearHighlighting();
            m_dataSetView2->clearHighlighting();

            m_dataSetView1->addHighlighting(file1_matches);
            m_dataSetView2->addHighlighting(file2_matches);
            m_dataSetView1->addDiffHighlighting(file1_differences);
            m_dataSetView2->addDiffHighlighting(file2_differences);


            m_dataSetView1->printByteGrid(ui->textEdit_dataSet1, ui->textEdit_address1);
            m_dataSetView2->printByteGrid(ui->textEdit_dataSet2, ui->textEdit_address2);
        }


    } else {

        if (!nextState) {
            LOG.Debug("!nextState");
        }
        else if (!nextState->newAlignmentRange) {
            LOG.Debug("!nextState->newAlignmentRange");
        }

    }
}

void MainWindow::on_actionDebug1_triggered()
{
    doSearchProcessing(searchProcessing::searchAction::advanceBothPointersBy1);
}

void MainWindow::on_actionDebug2_triggered()
{
    doSearchProcessing(searchProcessing::searchAction::advanceDataSet1PointerUntilMatchRange);
}

void MainWindow::on_actionDebug3_triggered()
{
    doSearchProcessing(searchProcessing::searchAction::advanceDataSet2PointerUntilMatchRange);
}

void MainWindow::on_actionDebug4_triggered()
{
    doSearchProcessing(searchProcessing::searchAction::advanceDataSet1PointerUntilAlignmentRange);
}

void MainWindow::on_actionDebug5_triggered()
{
    doSearchProcessing(searchProcessing::searchAction::advanceDataSet2PointerUntilAlignmentRange);
}

void MainWindow::on_actionReset_triggered()
{
    doSearchProcessing(searchProcessing::searchAction::none);
}
