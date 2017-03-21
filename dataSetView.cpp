#include "dataSetView.h"


dataSetView::highlightSet::highlightSet(QSharedPointer<QVector<byteRange>> ranges)
    :   m_ranges(ranges),
        m_applyForeground(false),
        m_applyBackground(false)
{

}

dataSetView::highlightSet::highlightSet()
    :   m_ranges(),
        m_applyForeground(false),
        m_applyBackground(false)
{

}

void dataSetView::highlightSet::setForegroundColor(const QColor &foreground)
{
    m_applyForeground = true;
    m_foreground = foreground;
}

void dataSetView::highlightSet::setBackgroundColor(const QColor &background)
{
    m_applyBackground = true;
    m_background = background;
}


dataSetView::dataSetView(QSharedPointer<dataSet>& theDataSet)
    : byteGridColumnMode(ByteGridColumnMode::Fill),
      byteGridColumn_LargestMultipleOf_N(8),
      byteGridColumn_UpTo_N(8),
      byteGridScrollingMode(ByteGridScrollingMode::FixedRows),
      m_dataSet(theDataSet),
      m_highlightSets(),
      m_subset(),
      m_bytesPerRow(0)
{
}

byteRange dataSetView::getSubset() const
{
    return m_subset;
}

void dataSetView::setSubset(byteRange subset)
{
    m_subset = subset;
    emit subsetChanged(m_subset);
}

unsigned int dataSetView::getSubsetStart() const
{
    return m_subset.start;
}

void dataSetView::setSubsetStart(unsigned int start)
{
    m_subset.start = start;
    emit subsetChanged(m_subset);
}

void dataSetView::addHighlightSet(const highlightSet& hSet)
{
    m_highlightSets.append(hSet);
}

unsigned int dataSetView::getBytesPerRow()
{
    return m_bytesPerRow;
}

void dataSetView::updateByteGridDimensions(QTextEdit* textEdit)
{
    //note: this code only works for MONOSPACE FONTS

    //no-op values: only update at the end if everything goes well
    m_bytesPerRow = 0;
    m_subset.count = 0;


    //calculate the pixel size of the draw area in the QTextEdit that is available
    //  to draw the byte grid:
    //  The drawable area is textEdit->viewport().
    //  Text starts a bit inside this area; we can see where by moving the cursor
    //  to the start of the control and measuring the cursor location.
    textEdit->moveCursor(QTextCursor::Start);
    QPoint topLeftCornerOfFirstChar = textEdit->cursorRect().topLeft();
    //
    int areaWidth_px  = textEdit->viewport()->width()  - topLeftCornerOfFirstChar.x();
    int areaHeight_px = textEdit->viewport()->height() - topLeftCornerOfFirstChar.y();
    //


    QFontMetrics qfm(textEdit->font());
    int byteWidth_px = qfm.width("00 ");    //width of one displayed byte value in pixels

    if (0 >= byteWidth_px) {return;}


    //calculate visible bytes per row
    unsigned int rowBytes;
    {
        int val = areaWidth_px/byteWidth_px;
        ASSERT_NOT_NEGATIVE(val);
        rowBytes = static_cast<unsigned int>(val);
    }

    //Decrease bytes per row if it won't fit in the draw area:
    //  this can happen because the single byte string width is slightly underreported
    //  (QFontMetrics::width returns int, so it may be truncating fractions)
    //
    //assemble a test string as long as a full row of bytes (again, assuming MONOSPACE FONTS)
    QString fullRow;
    for (unsigned int i = 0; i < rowBytes; ++i) {
        fullRow += "00 ";
    }
    //
    //get the pixel width of the full row string
    int rowWidth_px = qfm.width(fullRow);
    //reduce the row count by one byte until it fits
    while ((rowWidth_px >= areaWidth_px)
        && (rowBytes > 0))
    {
        rowWidth_px -= byteWidth_px;
        --rowBytes;
    }
    //

    if (0 >= rowBytes) {return;}

    //reduce bytes per row, if necessary, to implement ByteGridColumnMode
    switch (byteGridColumnMode)
    {
        case ByteGridColumnMode::Fill:
            //already filled to the limit, do nothing
            break;

        case ByteGridColumnMode::LargestMultipleOfN:
            if(byteGridColumn_LargestMultipleOf_N){
                rowBytes -= rowBytes%byteGridColumn_LargestMultipleOf_N;
            } else {
                LOG.Error("invalid byteGridColumn_LargestMultipleOf_N value");
            }

            if (0 == rowBytes) {
                LOG.Error("byteGridColumn_LargestMultipleOf_N value is greater than window width");
            }

            break;

        case ByteGridColumnMode::UpToN:
            if(rowBytes > byteGridColumn_UpTo_N) {
                rowBytes = byteGridColumn_UpTo_N;
            }
            break;

        case ByteGridColumnMode::LargestPowerOf2:
            {
                unsigned int val = 1;
                while (val*2 < rowBytes) {
                    val *= 2;
                }
                rowBytes = val;
            }
            break;

        case ByteGridColumnMode::LargestPowerOf2Extra:
            {
                unsigned int val = 1;
                while (val*2 < rowBytes) {
                    val *= 2;
                }

                //add an extra half width if it fits
                unsigned int withExtra = val + val/2;
                if (withExtra < rowBytes) {
                    val = withExtra;
                }
                rowBytes = val;
            }
            break;

        default:    //this should never happen
            LOG.Error("unhandled ByteGridColumnMode value");
    }

    if (0 >= rowBytes) {return;}


    //calculate total visible byte count

    int rowHeight_px;  //height of one displayed byte row in pixels
    //
    // rowHeight_px could be calculated with
    //      QFontMetrics::lineSpacing()
    //      QFontMetrics::height()
    // or   QFontMetrics::boundingRect(QString).height()
    //
    // (These all return ints)
    // None of these values reliably predict the pixel height of a number of rows through simple multiplication.
    // I chose QFontMetrics::boundingRect(QString).height() because it is typically >= the others,
    //  so error is likely to leave a row of screen space empty, instead of partially occluding the bottom row.
    //
    QRect qr = qfm.boundingRect("00_");
    rowHeight_px = qr.height();
    //

    if (0 >= rowHeight_px) {return;}
    if (0 >= areaHeight_px) {return;}

    int rowCount = areaHeight_px/rowHeight_px;
    ASSERT_NOT_NEGATIVE(rowCount);
    unsigned int byteCount = static_cast<unsigned int>(rowCount) * rowBytes;

    if (0 >= byteCount) {return;}


    m_bytesPerRow = rowBytes;
    m_subset.count = byteCount;

}

bool dataSetView::printByteGrid(QTextEdit* textEdit, QTextEdit* addressColumn)
{
    //clear any existing text
    //  (at the start of the function, so errors will show blank controls rather than stale data)
    textEdit->clear();
    addressColumn->clear();

    QSharedPointer<dataSet> theDataSet = m_dataSet.lock();
    //QSharedPointer<QVector<byteRange>> diffs = m_diffs.lock();

    //ensure that weak pointer lock succeeded
    if ( !theDataSet ){
        return false;
    }

    //skip no-print situations
    if (0 == m_bytesPerRow || 0 == m_subset.count) {
        return false;
    }


    QString displayText = "";   //text for the main data display area
    QString addressText = "";   //text for the address column area

    //get the data to be displayed
    Q_CHECK_PTR(theDataSet->getData());
    const QVector<unsigned char>& theData = *theDataSet->getData();

    unsigned int bytesPrinted = 0;
    ASSERT_LE_INT_MAX(m_subset.end());  //ensure static_cast<int>(i) in loop is safe
    for (unsigned int i = m_subset.start; i < m_subset.end(); i++) {

        if (static_cast<int>(i) >= theData.size()) {
            //break if we've exhausted the data
            //(this can happen if the file is smaller than the display area's capacity)
            break;
        }

        if (0 == bytesPrinted%m_bytesPerRow) {

            if (i != m_subset.start) {  //add newlines (unless starting the first row)
                displayText += "\n";
                addressText += "\n";
            }

            //list byte address for the start of this row
            //                              length 8, base 16, padded with '0's
            addressText += QString("0x%1  ").arg(i,8,16,QChar('0'));
        }

        //add the next byte
        if (!(theData[static_cast<int>(i)] & 0xF0)){
            displayText += "0"; //add a leading 0 for a most-significant half-byte of zero
        }
        displayText +=  QString::number(theData[static_cast<int>(i)], 16 ).toUpper() + " ";   //display in hex w/capital letters

        ++bytesPrinted;
    }

    //write strings to view areas
    textEdit->setTextColor(QColor::fromRgb(0,0,0));
    textEdit->insertPlainText(displayText);

    addressColumn->setTextColor(QColor::fromRgb(64,64,128));
    addressColumn->insertPlainText(addressText);

    //apply highlights for colored byte text regions
    for (highlightSet hSet : m_highlightSets)
    {
        highlightByteGrid(textEdit, hSet);
    }

    return true;
}

bool dataSetView::highlightByteGrid(QTextEdit* textEdit, highlightSet& hSet)
{
    //highlight/color byte text on the supplied ranges
    QList<QTextEdit::ExtraSelection> selectionList = textEdit->extraSelections();   //get the list of existing selections
    unsigned int rowWidth = m_bytesPerRow*3 + 1;

    //text highlighting helper function:
    //get the cursor index of the beginning of a byte's text
    auto getByteCursorIndex = [=](unsigned int byteIndex, bool endOfSelection = false)->int{
        unsigned int index = byteIndex - m_subset.start;     //index in currently displayed byte range
        unsigned int ret = (index/m_bytesPerRow)*rowWidth + 3*(index%m_bytesPerRow);

        //if this cursor index will be the end of a selection, don't select the newline at the end of a line
        //  (prevents cursor out-of-bounds qt complaint at end of displayed byte range,
        //  and looks nicer with colored backgrounds at the other line ends)
        if ( endOfSelection && (0 == (index%m_bytesPerRow)) && ret > 0) {
            ret -= 1;
        }
        ASSERT_LE_INT_MAX(ret);
        return static_cast<int>(ret); //return int for QTextCursor::setPosition
    };

    //use QTextEdit::ExtraSelections to highlight ranges
    for (byteRange& range : *hSet.m_ranges.data()) {

        if (range.end() <= m_subset.start
         || m_subset.end() <= range.start)
        {
            continue;   //skip ranges in addresses that aren't being displayed
        }

        QTextEdit::ExtraSelection selection;
        textEdit->moveCursor(QTextCursor::Start);
        selection.cursor = textEdit->textCursor();

        selection.cursor.setPosition(getByteCursorIndex(qMax(range.start, m_subset.start)        ),  QTextCursor::MoveAnchor);
        selection.cursor.setPosition(getByteCursorIndex(qMin(range.end(), m_subset.end()), true  ),  QTextCursor::KeepAnchor);

        if (hSet.m_applyForeground) {   //apply foreground color if specified
            selection.format.setForeground(hSet.m_foreground);
        }

        if (hSet.m_applyBackground) {   //apply background color if specified
            selection.format.setBackground(hSet.m_background);
        }

        selectionList.append(selection);
    }

    textEdit->setExtraSelections(selectionList);

    return true;
}

void dataSetView::addHighlighting(const std::multiset<blockMatchSet>& matches, bool useFirstDataSet)
{
    const unsigned char a = 48;
    const unsigned char B = 0;
    unsigned int currentColor = 0;
    std::vector<QColor> colorCycle = {
        QColor::fromRgb(a,B,B),
        QColor::fromRgb(B,a,B),
        QColor::fromRgb(B,B,a),

        QColor::fromRgb(B,a,a),
        QColor::fromRgb(a,B,a),
        QColor::fromRgb(a,a,B)
    };

    for (const blockMatchSet& match : matches) {

        const std::vector<unsigned int>& indices =
            useFirstDataSet ? match.data1_BlockStartIndices
                            : match.data2_BlockStartIndices;

        auto byteRanges = QSharedPointer<QVector<byteRange>>::create();

        for (auto& index : indices) {
            byteRanges.data()->append(byteRange(index,match.blockSize));
        }

        dataSetView::highlightSet hSet(byteRanges);
        hSet.setForegroundColor(QColor::fromRgb(128,128,128));
        hSet.setBackgroundColor(colorCycle[ currentColor++ % colorCycle.size() ]);
        addHighlightSet(hSet);

    }
}

void dataSetView::addHighlighting(const std::multiset<byteRange>& ranges)
{
    const unsigned char a = 128;
    const unsigned char B = 0;
    unsigned int currentColor = 0;
    std::vector<QColor> colorCycle = {
        QColor::fromRgb(a,B,B),
        QColor::fromRgb(B,a,B),
        QColor::fromRgb(B,B,a),

        QColor::fromRgb(B,a,a),
        QColor::fromRgb(a,B,a),
        QColor::fromRgb(a,a,B)
    };

    auto byteRanges = QSharedPointer<QVector<byteRange>>::create();

    for (const byteRange& range : ranges) {
        byteRanges->push_back(range);
    }

    dataSetView::highlightSet hSet(byteRanges);
    hSet.setForegroundColor(QColor::fromRgb(255,255,255));
    hSet.setBackgroundColor(colorCycle[ currentColor++ % colorCycle.size() ]);
    addHighlightSet(hSet);

}

void dataSetView::addDiffHighlighting(const std::list<byteRange>& ranges)
{
    const unsigned char a = 192;
    const unsigned char B = 255;
    unsigned int currentColor = 0;
    std::vector<QColor> colorCycle = {
        QColor::fromRgb(a,B,B),
        QColor::fromRgb(B,a,B),
        QColor::fromRgb(B,B,a),

        QColor::fromRgb(B,a,a),
        QColor::fromRgb(a,B,a),
        QColor::fromRgb(a,a,B)
    };

    for (const byteRange& range : ranges) {

        auto byteRanges = QSharedPointer<QVector<byteRange>>::create();
        byteRanges->push_back(range);

        dataSetView::highlightSet hSet(byteRanges);
        hSet.setForegroundColor(QColor::fromRgb(0,0,0));
        hSet.setBackgroundColor(colorCycle[ currentColor++ % colorCycle.size() ]);
        addHighlightSet(hSet);
    }
}

void dataSetView::clearHighlighting()
{
    m_highlightSets.clear();
}
