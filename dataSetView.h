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

public:
    // a set of byte ranges to highlight with text colors
    class highlightSet {
        friend class dataSetView;
    public:
        highlightSet(QSharedPointer<QVector<byteRange>> ranges);
        highlightSet();
        void setForegroundColor(const QColor &foreground);
        void setBackgroundColor(const QColor &background);

    private:
        QSharedPointer<QVector<byteRange>> m_ranges;
        bool m_applyForeground;
        bool m_applyBackground;
        QColor m_foreground;
        QColor m_background;
    };

public:
    dataSetView(QSharedPointer<dataSet>& theDataSet);

    void updateByteGridDimensions(QTextEdit* textEdit);
    bool printByteGrid(QTextEdit* textEdit, QTextEdit* addressColumn);

    //gets/sets the subset of the dataSet being displayed
    byteRange getSubset() const;
    void setSubset(byteRange subset);

    //gets/sets the start index of the dataSet being displayed
    unsigned int getSubsetStart() const;
    void setSubsetStart(unsigned int start);

    void addHighlightSet(const highlightSet& hSet);

signals:
    void subsetChanged(byteRange subset);

private:
    bool highlightByteGrid(QTextEdit* textEdit, highlightSet& hSet);

    QWeakPointer<dataSet> m_dataSet;            //the dataSet that this dataSetView will display
    QVector<highlightSet> m_highlightSets;	//highlight regions which color the text
    byteRange m_subset;                         //the subset of the dataSet that is displayed by this dataSetView
    unsigned int m_bytesPerRow;                 //bytes per row in byte display grid

};

#endif // DATASETVIEW_H
