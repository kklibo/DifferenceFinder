#ifndef DATASETVIEW_H
#define DATASETVIEW_H

#include <QSharedPointer>
#include <QWeakPointer>
#include <QVector>
#include <QString>
#include <QTextEdit>
#include <QtGlobal>

#include <set>

#include "dataSet.h"
#include "indexrange.h"
#include "defensivecoding.h"
#include "blockmatchset.h"

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
        highlightSet(QSharedPointer<QVector<indexRange>> ranges);
        highlightSet();
        void setForegroundColor(const QColor &foreground);
        void setBackgroundColor(const QColor &background);

    private:
        QSharedPointer<QVector<indexRange>> m_ranges;
        bool m_applyForeground;
        bool m_applyBackground;
        QColor m_foreground;
        QColor m_background;
    };

    //mode for setting the number of columns in the byte grid
    enum class ByteGridColumnMode
    {
        Fill,                   //fills the window with as many columns as possible (default)
        LargestMultipleOfN,     //
        LargestPowerOf2,        //
        UpToN,                  //as many columns as possible, up to a max of n
        LargestPowerOf2Extra    //largest ((largest power of 2)*(1 or 1.5)) that will fit
    };

    ByteGridColumnMode byteGridColumnMode;
    unsigned int byteGridColumn_LargestMultipleOf_N;
    unsigned int byteGridColumn_UpTo_N;

    //view range adjustment mode for scrolling
    enum class ByteGridScrollingMode
    {
        FixedRows,      //row start addresses are preserved through scrolling (default)
        Free            //continuous scrolling, row starts can have any address
    };
    ByteGridScrollingMode byteGridScrollingMode;


    dataSetView(QSharedPointer<dataSet>& theDataSet);

    void updateByteGridDimensions(QTextEdit* textEdit);
    bool printByteGrid(QTextEdit* textEdit, QTextEdit* addressColumn);

    //gets the subset of the dataSet being displayed
    indexRange getSubset() const;

    //gets/sets the start index of the dataSet being displayed
    unsigned int getSubsetStart() const;
    void setSubsetStart(unsigned int start);

    void addHighlightSet(const highlightSet& hSet);

    void addHighlighting(const std::multiset<blockMatchSet>& matches, bool useFirstDataSet);
    void addHighlighting(const std::multiset<indexRange>& ranges);
    void addHighlighting(const std::list<indexRange>& ranges);
    void addDiffHighlighting(const std::list<indexRange>& ranges);
    void addByteColorHighlighting();
    void clearHighlighting();

    unsigned int getBytesPerRow();

signals:
    void subsetChanged(indexRange subset);   //was used for debugging dataSetView, should this be removed?

private:
    bool highlightByteGrid(QTextEdit* textEdit, highlightSet& hSet);

    QWeakPointer<dataSet> m_dataSet;            //the dataSet that this dataSetView will display
    QVector<highlightSet> m_highlightSets;      //highlight regions which color the text
    indexRange m_subset;                        //the subset of the dataSet that is displayed by this dataSetView
    unsigned int m_bytesPerRow;                 //bytes per row in byte display grid

};

//declare these enums for Qt's meta-object system so they can be stored in QVariants
//(for convenient labeling of combo box items in the settings dialog)
Q_DECLARE_METATYPE(dataSetView::ByteGridColumnMode)
Q_DECLARE_METATYPE(dataSetView::ByteGridScrollingMode)

#endif // DATASETVIEW_H
