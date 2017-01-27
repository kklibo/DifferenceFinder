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

    if (m_subset.end() > (unsigned int)theDataSet->getData()->size()) {
        return false;
    }

    int ndiff = 0;  //the index of the current difference byteRange, for colored display

    //why is this needed?
    QColor orig = textEdit->textColor();
    textEdit->setTextColor(orig);


    while (diffs->data()[ndiff].end() <= m_subset.start && ndiff < diffs->size())
    {
        ndiff++;    //advance through diff ranges to the first one that either applies to this subset or starts after it
    }

    QString displayText = "";
    QString addressText = "";
    //reserve string memory here?
    int rowBytes = 32;//assumed nonzero, add check if this changes
    int rowsPrinted = 0;
    int byteCount = 0;
    for (unsigned int i = m_subset.start; i < m_subset.end(); i++) {

        QString str2;
        //list byte address at the start of each row
        if (0 == byteCount%rowBytes) {
        //    textEdit->setTextColor(QColor::fromRgb(64,64,128));

            if (i != m_subset.start) {
                str2 += "\n";
            }
            //                      length 8, base 16, padded with '0's
            str2 += QString("0x%1  ").arg(i,8,16,QChar('0'));

            //textEdit->insertPlainText(str2);
            displayText += str2;
            addressText += str2;
            ++rowsPrinted;
        //    textEdit->setTextColor(orig);
        }
        ++byteCount;

        QString str;
        //show a row of bytes
        Q_CHECK_PTR(theDataSet->getData());
        QVector<unsigned char>& theData = *theDataSet->getData();
        if (!(theData[i] & 0xF0)){
            str += "0"; //add a leading 0
        }
        str +=  QString::number(theData[i], 16 ).toUpper() + " ";   //display in hex w/capital letters
/*
        if (ndiff < diffs->size()) {    //if we haven't gone past the last difference range

            //if we are inside the current difference range, draw red text
            if (i >= diffs->data()[ndiff].start) {
        //        textEdit->setTextColor(QColor::fromRgb(255,0,0));
            }

            //if we have finished the current difference range, reset text color and go to next range
            if (i >= diffs->data()[ndiff].end()) {
        //        textEdit->setTextColor(orig);
                ndiff++;
            }
        }*/

        //textEdit->insertPlainText(str); //scrollbar lag comes from this
        displayText += str;
    }

    //textEdit->setTextColor(orig);
    textEdit->insertPlainText(displayText);
/*
    QFontMetrics qfm(addressColumn->font());
    QRect qr = qfm.boundingRect("0x00000000 _");

    addressColumn->setFixedWidth(qr.width());
    addressColumn->setTextColor(QColor::fromRgb(64,64,128));*/
    addressColumn->insertPlainText(addressText);
    //textEdit->setReadOnly(false);



    QList<QTextEdit::ExtraSelection> selectionList;
    constexpr int addressTextLength = 2 + 8;
    int rowWidth = addressTextLength + 2 + rowBytes*3 + 1;

    auto getCursorIndex = [=](int row, int column)->int{
        return row*rowWidth + column;
    };

    auto getByteCursorIndex = [=](int byteIndex)->int{
        int index = byteIndex - m_subset.start;
//        return (index/rowBytes + 1)*(addressTextLength + 2) + 3*(index%rowBytes) ;//+ (index/rowBytes)*1;
        return (index/rowBytes)*rowWidth + addressTextLength + 2 + 3*(index%rowBytes);
    };

    //use QTextEdit::ExtraSelections to change address text color
    for (int i = 0; i < rowsPrinted; i++) {
        QTextEdit::ExtraSelection selection;

        //reset text cursor
        textEdit->moveCursor(QTextCursor::Start);
        selection.cursor = textEdit->textCursor();

        //select address text and set color
        selection.cursor.setPosition(getCursorIndex(i, 0),                  QTextCursor::MoveAnchor);
        selection.cursor.setPosition(getCursorIndex(i, addressTextLength),  QTextCursor::KeepAnchor);
        //selection.cursor.setPosition(i*rowWidth,                        QTextCursor::MoveAnchor);
        //selection.cursor.setPosition(i*rowWidth + addressTextLength,    QTextCursor::KeepAnchor);
        selection.format.setForeground(QColor::fromRgb(0,255,0));//64,64,128));

        selectionList.append(selection);
    }

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

        selection.cursor.setPosition(getByteCursorIndex(qMax(diff.start, m_subset.start)),  QTextCursor::MoveAnchor);
        selection.cursor.setPosition(getByteCursorIndex(qMin(diff.end(), m_subset.end())),  QTextCursor::KeepAnchor);
        //selection.format.setForeground(QColor::fromRgb(255,0,0));
        selection.format.setBackground(QColor::fromRgb(255,128,128));
        selectionList.append(selection);
    //    break;

    }



/*
    QTextEdit::ExtraSelection selection;
    textEdit->moveCursor(QTextCursor::Start);
    selection.cursor = textEdit->textCursor();

    selection.cursor.setPosition(getByteCursorIndex(1319),                  QTextCursor::MoveAnchor);
    selection.cursor.setPosition(getByteCursorIndex(1320),  QTextCursor::KeepAnchor);
    selection.format.setForeground(QColor::fromRgb(0,255,0));
    selectionList.append(selection);
*/


    textEdit->setExtraSelections(selectionList);



/*
    QList<QTextEdit::ExtraSelection> testList;
    QTextEdit::ExtraSelection test1;

    textEdit->moveCursor(QTextCursor::Start);
    test1.cursor = textEdit->textCursor();
    test1.cursor.setPosition(12, QTextCursor::MoveAnchor);
    test1.cursor.setPosition(30, QTextCursor::KeepAnchor);

    //test1.cursor.clearSelection();
    test1.format.setBackground(QColor(Qt::yellow));
    test1.format.setForeground(QColor(Qt::red));
    //test1.format.setProperty(QTextFormat::FullWidthSelection, true);
    testList.append(test1);

    textEdit->setExtraSelections(testList);

    //textEdit->setReadOnly(true);
*/
    return true;
}
