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

bool dataSetView::vectorSubsetToQTextEdit(QTextEdit* textEdit)
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

    QColor orig = textEdit->textColor();



    while (diffs->data()[ndiff].end() <= m_subset.start && ndiff < diffs->size())
    {
        ndiff++;    //advance through diff ranges to the first one that either applies to this subset or starts after it
    }

    int rowBytes = 12;
    int byteCount = 0;
    for (unsigned int i = m_subset.start; i < m_subset.end(); i++) {

        QString str2;
        //list byte address at the start of each row
        if (0 == byteCount%rowBytes) {
            textEdit->setTextColor(QColor::fromRgb(64,64,128));

            if (i != m_subset.start) {
                str2 += "\n";
            }
            str2 += QString("0x%1  ").arg(i,8,16,QChar('0'));

            textEdit->insertPlainText(str2);
            textEdit->setTextColor(orig);
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

        if (ndiff < diffs->size()) {    //if we haven't gone past the last difference range

            //if we are inside the current difference range, draw red text
            if (i >= diffs->data()[ndiff].start) {
                textEdit->setTextColor(QColor::fromRgb(255,0,0));
            }

            //if we have finished the current difference range, reset text color and go to next range
            if (i >= diffs->data()[ndiff].end()) {
                textEdit->setTextColor(orig);
                ndiff++;
            }
        }

        textEdit->insertPlainText(str);
    }

    textEdit->setTextColor(orig);
    return true;
}
