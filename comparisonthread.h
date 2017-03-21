#ifndef COMPARISONTHREAD_H
#define COMPARISONTHREAD_H

#include <QThread>
#include <QColor>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>

#include <vector>
#include <queue>
#include <set>
#include <memory>
#include <utility>

#include "comparison.h"
#include "dataSet.h"

class comparisonThread : public QThread
{
    Q_OBJECT

public:
    comparisonThread(   QObject* parent = nullptr   );
    ~comparisonThread();

    void doCompare();

    QSharedPointer<std::multiset<blockMatchSet>> getResultMatches();
    QSharedPointer<std::list<byteRange>> getData1_unmatchedBlocks();
    QSharedPointer<std::list<byteRange>> getData2_unmatchedBlocks();

    void setDataSet1(QSharedPointer<dataSet> dataSet1);
    void setDataSet2(QSharedPointer<dataSet> dataSet2);


signals:
    void sendMessage(QString message, QColor color);
    void resultsAreReady();


protected:
    void run() override;


private:
    QMutex m_mutex;
    bool m_abort;
    bool m_resultsReady;

    //inputs
    QSharedPointer<dataSet> m_dataSet1;
    QSharedPointer<dataSet> m_dataSet2;

    //outputs
    QSharedPointer<std::multiset<blockMatchSet>> m_resultMatches;
    QSharedPointer<std::list<byteRange>> m_result_data1_unmatchedBlocks;
    QSharedPointer<std::list<byteRange>> m_result_data2_unmatchedBlocks;

};

#endif // COMPARISONTHREAD_H
