#ifndef DATASETVIEW_H
#define DATASETVIEW_H

#include <QSharedPointer>
#include <QWeakPointer>
#include <QVector>
#include <QString>

#include "dataSet.h"
#include "bincomp.h"

/*
 *
*/

class dataSetView : public QObject
{

    Q_OBJECT

public:
    dataSetView(QSharedPointer<dataSet>& theDataSet);
    dataSetView(QSharedPointer<dataSet>& theDataSet, QSharedPointer<QVector<byterange>>& diffs);
    bool vectorSubsetToQTextEdit(QTextEdit* textEdit);

    byterange getSubset() const;
    void setSubset(byterange subset);

    int getSubsetStart() const;
    void setSubsetStart(int start);

signals:
    void subsetChanged(byterange subset);

private:
    QWeakPointer<dataSet> m_dataSet;
    QWeakPointer<QVector<byterange>> m_diffs;
    byterange m_subset;

};

#endif // DATASETVIEW_H
