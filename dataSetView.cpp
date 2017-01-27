#include "dataSetView.h"

dataSetView::dataSetView(QSharedPointer<dataSet>& theDataSet)
{
    m_dataSet = theDataSet;
}

dataSetView::dataSetView(QSharedPointer<dataSet>& theDataSet, QSharedPointer<QVector<byteRange>>& diffs)
{
    m_dataSet = theDataSet;
    m_diffs = diffs;
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

bool dataSetView::vectorSubsetToQTextEdit(QTextEdit* textEdit, QTextEdit* addressColumn)
{

    QSharedPointer<dataSet> theDataSet = m_dataSet.lock();
    if (!theDataSet){
        return false;
    }

    QSharedPointer<QVector<byteRange>> diffs = m_diffs.lock();
    if (!diffs){
        return false;
    }

    QString displayText = "";   //text for the main data display area
    QString addressText = "";   //text for the address column area
        //if performance drops from this function, reserve string memory for these?

    //get the data to be displayed
    Q_CHECK_PTR(theDataSet->getData());
    QVector<unsigned char>& theData = *theDataSet->getData();

    const int rowBytes = 32;//assumed nonzero, add check if this changes
    int bytesPrinted = 0;
    for (unsigned int i = m_subset.start; i < m_subset.end(); i++) {

        if (static_cast<int>(i) >= theData.size()) {
            //break if we've exhausted the data
            //(this can happen if the file is smaller than the display area's capacity)
            break;
        }

        if (0 == bytesPrinted%rowBytes) {

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
    int rowWidth = rowBytes*3 + 1;

    //text highlighting helper function:
    //get the cursor index of the beginning of a byte's text
    auto getByteCursorIndex = [=](int byteIndex, bool endOfSelection = false)->int{
        int index = byteIndex - m_subset.start;     //index in currently displayed byte range
        int ret = (index/rowBytes)*rowWidth + 3*(index%rowBytes);

        //if this cursor index will be the end of a selection, don't select the newline at the end of a line
        //  (prevents cursor out-of-bounds qt complaint at end of displayed byte range,
        //  and looks nicer with colored backgrounds at the other line ends)
        if ( endOfSelection && (0 == (index%rowBytes)) ) {
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
