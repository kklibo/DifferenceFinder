#ifndef DATASETVIEW_H
#define DATASETVIEW_H

#include <QSharedPointer>
#include <QWeakPointer>
#include <QVector>
#include <QString>
#include <QTextEdit>
#include <QtGlobal>

#include "dataSet.h"
#include "byteRange.h"

/*
    displays a dataSet in the QT interface
*/

class dataSetView : public QObject
{

    Q_OBJECT

private:
    // a set byte ranges and text highlighting colors
    struct highlightSet {
        QSharedPointer<QVector<byteRange>> ranges;
        QColor *foreground = nullptr;
        QColor *background = nullptr;
    };

public:
    dataSetView(QSharedPointer<dataSet>& theDataSet);
    dataSetView(QSharedPointer<dataSet>& theDataSet, QSharedPointer<QVector<byteRange>>& diffs);

    void updateByteGridDimensions(QTextEdit* textEdit);
    bool printByteGrid(QTextEdit* textEdit, QTextEdit* addressColumn);

    //gets/sets the subset of the dataSet being displayed
    byteRange getSubset() const;
    void setSubset(byteRange subset);

    //gets/sets the start index of the dataSet being displayed
    unsigned int getSubsetStart() const;
    void setSubsetStart(unsigned int start);

private:
    bool highlightByteGrid(QTextEdit* textEdit, QVector<byteRange>& ranges, QColor *foreground = nullptr, QColor *background = nullptr);

signals:
    void subsetChanged(byteRange subset);

private:
    QWeakPointer<dataSet> m_dataSet;            //the dataSet that this dataSetView will display
    QWeakPointer<QVector<byteRange>> m_diffs;   //vector of byteRanges to mark as differences
    byteRange m_subset;                         //the subset of the dataSet that is displayed by this dataSetView
    unsigned int m_bytesPerRow;                 //bytes per row in byte display grid

};

#endif // DATASETVIEW_H
