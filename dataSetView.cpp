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
    //no-op values: only update at the end if everything goes well
    m_bytesPerRow = 0;
    m_subset.count = 0;

    //calculate the size of one displayed byte value in the current font
    QFontMetrics qfm(textEdit->font());
    QRect qr = qfm.boundingRect("00_"); //this might be the wrong size, leaving it for now

    int byteWidth_px = qfm.width("0 0");    //width of one displayed byte value in pixels

    //calculate visible bytes per row

    //this causes an unwanted font color change in textEdit
    //  and this function seems to work without it.
    //
    //  leaving it out for now but it should probably be added eventually
    //  to ensure that the cursor rectangle we use to measure the top and left margins
    //  is actually at the top left of the control
    //
    textEdit->moveCursor(QTextCursor::Start);

    QPoint topLeftCornerOfFirstChar = textEdit->cursorRect().topLeft();


//    int bytePixelWidth = qr.width();
   // int areaWidth_px = textEdit->width() - topLeftCornerOfFirstChar.x();

     //int areaWidth_px = textEdit->contentsRect().width() - topLeftCornerOfFirstChar.x();
    int areaWidth_px = textEdit->viewport()->width() - topLeftCornerOfFirstChar.x();

    if(textEdit->contentsRect().width() != textEdit->viewport()->width())
    LOG.sendMessage(QStringLiteral("%1 %2")
                    .arg(textEdit->contentsRect().width())
                    .arg(textEdit->viewport()->width())
                    , QColor::fromRgb(0,0,128));
/*
    LOG.sendMessage(QStringLiteral("%1 %2").arg(textEdit->width()).arg(textEdit->viewport()->width()), QColor::fromRgb(0,128,0));
    LOG.sendMessage(QStringLiteral("%1 %2  %3")
                    .arg(textEdit->contentsMargins().right())
                    .arg(textEdit->contentsMargins().left())
                    .arg(textEdit->contentsMargins().right() - textEdit->contentsMargins().left())
                    , QColor::fromRgb(128,0,0));
    LOG.sendMessage(QStringLiteral("%1")
                    .arg(textEdit->contentsRect().width())
                    , QColor::fromRgb(128,128,0));
    LOG.sendMessage(QStringLiteral("%1 %2  %3 %4")
                    .arg(textEdit->cursorRect().topLeft().x())
                    .arg(textEdit->cursorRect().topLeft().y())
                    .arg(textEdit->cursorRect().bottomRight().x())
                    .arg(textEdit->cursorRect().bottomRight().y())
                    , QColor::fromRgb(0,128,128));
    LOG.sendMessage(QStringLiteral("%1 %2 %3 %4 %5 %6 %7 %8 %9")
                    .arg(qfm.width("00 "))
                    .arg(qfm.width("0 0"))
                    .arg(qfm.width("00_"))
                    .arg(qfm.width("0"))
                    .arg(qfm.width(" "))
                    .arg(qfm.width("00 00 00 00 00 00 00 00 00 00 "))
                    .arg(qfm.width("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "))
                    .arg(qfm.width("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "))
                    .arg(qfm.width("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "))
                    , QColor::fromRgb(128,0,128));
*/

    if (0 >= byteWidth_px) {return;}

    LOG.sendMessage(QStringLiteral("areaWidth_px: %1   byteWidth_px: %2   areaWidth_px/byteWidth_px: %3")
                    .arg(areaWidth_px)
                    .arg(byteWidth_px)
                    .arg(areaWidth_px/byteWidth_px)
                    , QColor::fromRgb(128,0,0));
    LOG.sendMessage(QStringLiteral("15 byte width: %1")
                    .arg(qfm.width("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "))
                    , QColor::fromRgb(128,128,0));

    //int rowBytes = ( areaPixelWidth-(bytePixelWidth/2) )/bytePixelWidth;
    int rowBytes = areaWidth_px/byteWidth_px;

    //decrease bytes per row if it's too big
    //  this can happen because the single byte string width is slightly underreported
    //  (QFontMetrics::width returns int, so it may truncate a fraction)
    QString fullLine;
    for (int i = 0; i < rowBytes; ++i) {
        fullLine += "00 ";
    }

    {
        int rowWidth_px = qfm.width(fullLine);
        int viewportWidth_px = areaWidth_px;//textEdit->viewport()->width();
        while ((rowWidth_px >= viewportWidth_px)
            && (rowBytes > 0))
        {
            rowWidth_px -= byteWidth_px;
            --rowBytes;
        }

        LOG.sendMessage(QStringLiteral("rowWidth_px: %1   viewportWidth_px: %2   rowBytes(adjusted): %3")
                        .arg(rowWidth_px)
                        .arg(viewportWidth_px)
                        .arg(rowBytes)
                        , QColor::fromRgb(0,128,0));


    }



    if (0 >= rowBytes) {return;}

    //calculate total visible byte count
    //int bytePixelHeight = qr.height();
    //int areaPixelHeight = textEdit->height();

    int byteHeight_px = qr.height();//qfm.lineSpacing();//qfm.height(); qr.height()   //height of one displayed byte value in pixels
    int areaHeight_px = textEdit->viewport()->height() - topLeftCornerOfFirstChar.y();

    if (0 >= byteHeight_px) {return;}
    if (0 >= areaHeight_px) {return;}

    //the "-(byteHeight_px/2)" rounds the row count down so we don't draw a last row that only partially fits
   // int byteCount = ( (areaHeight_px-(byteHeight_px/2) )/byteHeight_px) * rowBytes;

     int byteCount = ( (areaHeight_px )/byteHeight_px) * rowBytes;

    if (0 >= byteCount) {return;}


    LOG.sendMessage(QStringLiteral("qr.height(): %1   qfm.height(): %2   qfm.lineSpacing(): %3   areaHeight_px: %4")
                    .arg(qr.height())
                    .arg(qfm.height())
                    .arg(qfm.lineSpacing())
                    .arg(areaHeight_px)
                    , QColor::fromRgb(0,128,128));

    LOG.sendMessage(QStringLiteral("rowBytes: %1   areaHeight_px/byteHeight_px: %2   byteCount: %3")
                  .arg(rowBytes)
                  .arg(areaHeight_px/byteHeight_px)
                  .arg(byteCount)
                  , QColor::fromRgb(255,128,128));
        //these are not always identical

    QString fullColumn = "0";
    for (int i = 1; i < 20; ++i) {
        fullColumn += "\n0";
    }
    QRect huge(-10000, -10000, 20000,20000);
    //QRect columnSize = qfm.boundingRect(fullColumn);
    QRect columnSize = qfm.boundingRect(huge, 0, fullColumn);
    LOG.sendMessage(QStringLiteral("20 rows height: %1")
                  .arg(columnSize.height())
                  , QColor::fromRgb(128,255,128));
    //use this to fix vertical sizing

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
    if (!theDataSet){
        return false;
    }

    QSharedPointer<QVector<byteRange>> diffs = m_diffs.lock();
    if (!diffs){
        return false;
    }

   // updateByteGridDimensions(textEdit);

    if (0 == m_bytesPerRow) {
        return false;
    }

    if (0 == m_subset.count) {
        return false;
    }
/*
//move this to different function? call with scrollbar sizing?
    QFontMetrics qfm(textEdit->font());
    QRect qr = qfm.boundingRect("00_");

    //calculate visible bytes per row
    int bytePixelWidth = qr.width();
    int areaPixelWidth = textEdit->width();

    if (0 >= bytePixelWidth) {
        return false;
    }

    int rowBytes = areaPixelWidth/bytePixelWidth;

    if (0 >= rowBytes) {
        return false;
    }

    //calculate total visible byte count
    int bytePixelHeight = qr.height();
    int areaPixelHeight = textEdit->height();

    if (0 >= bytePixelHeight) {
        return false;
    }

    //m_subset.count = (areaPixelHeight/bytePixelHeight) * rowBytes;
    m_subset.count = ((areaPixelHeight-(bytePixelHeight/2))/bytePixelHeight) * rowBytes;

    if (0 == m_subset.count) {
        return false;
    }
*/
    //prevent misaligned/hidden address text: make sure address column area uses the same font
  //  addressColumn->setFont(textEdit->font());

    QString displayText = "";   //text for the main data display area
    QString addressText = "";   //text for the address column area
        //if performance drops from this function, reserve string memory for these?

    //get the data to be displayed
    Q_CHECK_PTR(theDataSet->getData());
    QVector<unsigned char>& theData = *theDataSet->getData();

 //   const int rowBytes = 32;//assumed nonzero, add check if this changes
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
