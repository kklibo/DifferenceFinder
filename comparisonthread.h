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
#include "offsetmetrics.h"
#include "dataSet.h"

class comparisonThread : public QThread
{
    Q_OBJECT

public:

    enum class comparisonAlgorithm {
        largestBlock,
        sequential
    };

    comparisonThread(   QObject* parent = nullptr   );
    ~comparisonThread();

    bool startThread(comparisonAlgorithm algorithm);
    void abort();

    std::unique_ptr<   comparison::results> getResults_largestBlock();
    std::unique_ptr<offsetMetrics::results> getResults_sequential();

    void setDataSet1(QSharedPointer<dataSet> dataSet1);
    void setDataSet2(QSharedPointer<dataSet> dataSet2);


signals:
    void sendMessage(QString message, QColor color);    //for displaying log messages


protected:
    void run() override;


private:
    QMutex m_mutex;

    //special lock so abort() can be called while the thread holds m_mutex
    QMutex m_comparisonAlgorithmWriteLock;

    //comparison algorithm to use
    comparisonAlgorithm m_comparisonAlgorithm;

    //inputs
    QSharedPointer<dataSet> m_dataSet1;
    QSharedPointer<dataSet> m_dataSet2;

    //output
    std::unique_ptr<   comparison::results> m_results_largestBlock;
    std::unique_ptr<offsetMetrics::results> m_results_sequential;

};

#endif // COMPARISONTHREAD_H
