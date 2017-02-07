#include "dataSetView.h"

dataSetView::dataSetView(QSharedPointer<dataSet>& theDataSet)
{
    m_dataSet = theDataSet;
    m_bytesPerRow = 0;
}

dataSetView::dataSetView(QSharedPointer<dataSet>& theDataSet, QSharedPointer<QVector<byteRange>>& diffs)
{
    m_dataSet = theDataSet;
    m_diffs = diffs;
    m_bytesPerRow = 0;
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
    int rowBytes = areaWidth_px/byteWidth_px;

    //Decrease bytes per row if it won't fit in the draw area:
    //  this can happen because the single byte string width is slightly underreported
    //  (QFontMetrics::width returns int, so it may be truncating fractions)
    //
    //assemble a test string as long as a full row of bytes (again, assuming MONOSPACE FONTS)
    QString fullRow;
    for (int i = 0; i < rowBytes; ++i) {
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
    int byteCount = rowCount * rowBytes;

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
    QVector<unsigned char>& theData = *theDataSet->getData();

    int bytesPrinted = 0;
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
        if (!(theData[i] & 0xF0)){
            displayText += "0"; //add a leading 0 for a most-significant half-byte of zero
        }
        displayText +=  QString::number(theData[i], 16 ).toUpper() + " ";   //display in hex w/capital letters

        ++bytesPrinted;
    }

    //write strings to view areas
    textEdit->insertPlainText(displayText);
    addressColumn->insertPlainText(addressText);



    QSharedPointer<QVector<byteRange>> diffs = m_diffs.lock();
    //ensure that weak pointer lock succeeded
    if ( !diffs ) {
        return false;
    }

    //highlight displayed byte differences between files
    QList<QTextEdit::ExtraSelection> selectionList;
    int rowWidth = m_bytesPerRow*3 + 1;

    //text highlighting helper function:
    //get the cursor index of the beginning of a byte's text
    auto getByteCursorIndex = [=](int byteIndex, bool endOfSelection = false)->int{
        int index = byteIndex - m_subset.start;     //index in currently displayed byte range
        int ret = (index/m_bytesPerRow)*rowWidth + 3*(index%m_bytesPerRow);

        //if this cursor index will be the end of a selection, don't select the newline at the end of a line
        //  (prevents cursor out-of-bounds qt complaint at end of displayed byte range,
        //  and looks nicer with colored backgrounds at the other line ends)
        if ( endOfSelection && (0 == (index%m_bytesPerRow)) ) {
            ret -= 1;
        }
        return ret;
    };

    //use QTextEdit::ExtraSelections to highlight differences
    for (byteRange& diff : *diffs) {

        if (diff.end() <= m_subset.start
         || m_subset.end() <= diff.start)
        {
            continue;   //skip diffs in addresses that aren't being displayed
        }

        QTextEdit::ExtraSelection selection;
        textEdit->moveCursor(QTextCursor::Start);
        selection.cursor = textEdit->textCursor();

        selection.cursor.setPosition(getByteCursorIndex(qMax(diff.start, m_subset.start)        ),  QTextCursor::MoveAnchor);
        selection.cursor.setPosition(getByteCursorIndex(qMin(diff.end(), m_subset.end()), true  ),  QTextCursor::KeepAnchor);
        //selection.format.setForeground(QColor::fromRgb(255,0,0));
        selection.format.setBackground(QColor::fromRgb(255,128,128));
        selectionList.append(selection);
    }

    textEdit->setExtraSelections(selectionList);

    return true;
}
