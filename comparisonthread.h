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
    void abort();

    std::unique_ptr<comparison::results> getResults();

    void setDataSet1(QSharedPointer<dataSet> dataSet1);
    void setDataSet2(QSharedPointer<dataSet> dataSet2);


signals:
    void sendMessage(QString message, QColor color);    //for displaying log messages


protected:
    void run() override;


private:
    QMutex m_mutex;

    //inputs
    QSharedPointer<dataSet> m_dataSet1;
    QSharedPointer<dataSet> m_dataSet2;

    //output
    std::unique_ptr<comparison::results> m_results;

};

#endif // COMPARISONTHREAD_H
