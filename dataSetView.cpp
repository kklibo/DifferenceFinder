#include "dataSetView.h"

dataSetView::dataSetView(QSharedPointer<dataSet>& theDataSet)
{
    m_dataSet = theDataSet;
}

dataSetView::dataSetView(QSharedPointer<dataSet>& theDataSet, QSharedPointer<QVector<byterange>>& diffs)
{
    m_dataSet = theDataSet;
    m_diffs = diffs;
}

byterange dataSetView::getSubset() const
{
    return m_subset;
}

void dataSetView::setSubset(byterange subset)
{
    m_subset = subset;
    emit subsetChanged(m_subset);
}

int dataSetView::getSubsetStart() const
{
    return m_subset.start;
}

void dataSetView::setSubsetStart(int start)
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

    QSharedPointer<QVector<byterange>> diffs = m_diffs.lock();
    if (!diffs){
        return false;
    }

    if (m_subset.start < 0 || m_subset.end() > theDataSet->getData()->size()) {
        return false;
    }

    int ndiff = 0;

    QColor orig = textEdit->textColor();



    while (diffs->data()[ndiff].end() <= m_subset.start && ndiff < diffs->size())
    {
        ndiff++;    //advance through diffs to the first one that applies to this range
    }

    int rowBytes = 10;
    int byteCount = 0;
    for (int i = m_subset.start; i < m_subset.end(); i++) {

        QString str2;
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
        //str.reserve(65536);

        QVector<unsigned char>* theData = theDataSet->getData();
        if (!((*theData)[i] & 0xF0)){
       // if (!((*theDataSet.data())[i] & 0xF0)){
        //if (!(theDataSet->getData()[i] & 0xF0)){
            str += "0";
        }
        str +=  QString::number((*theData)[i], 16 ).toUpper() + " ";
        //str +=  QString::number(theDataSet->getData()[i], 16 ).toUpper() + " ";

        if (ndiff < diffs->size()) {

            if (i >= diffs->data()[ndiff].start) {
                textEdit->setTextColor(QColor::fromRgb(255,0,0));
            }

            if (i >= diffs->data()[ndiff].start + diffs->data()[ndiff].count) {
                textEdit->setTextColor(orig);
                ndiff++;
            }
        }

        textEdit->insertPlainText(str);
    }

    textEdit->setTextColor(orig);
    return true;
}
