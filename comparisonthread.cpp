#include "comparisonthread.h"

comparisonThread::comparisonThread(QObject* parent/*= nullptr*/)
  : QThread (parent),
    m_mutex(),
    m_comparisonAlgorithmWriteLock(),
    m_comparisonAlgorithm(comparisonAlgorithm::largestBlock),
    m_dataSet1(nullptr),
    m_dataSet2(nullptr),
    m_results_largestBlock(nullptr),
    m_results_sequential(nullptr)
{

}

comparisonThread::~comparisonThread()
{
    abort();
    wait(); //returns when run() is not running
}

bool comparisonThread::startThread(comparisonAlgorithm algorithm)
{
    QMutexLocker lock(&m_mutex);
    QMutexLocker lock2(&m_comparisonAlgorithmWriteLock);
    m_comparisonAlgorithm = algorithm;

    start();

    return true;
}

void comparisonThread::abort()
{
    QMutexLocker lock(&m_comparisonAlgorithmWriteLock);

    switch (m_comparisonAlgorithm) {

        case comparisonAlgorithm::largestBlock:
            comparison::abort();
            break;

        case comparisonAlgorithm::sequential:
            offsetMetrics::abort();
            break;

        default:
            FAIL();
    }
}

std::unique_ptr<comparison::results> comparisonThread::getResults_largestBlock()
{
    QMutexLocker lock(&m_mutex);

    return isFinished() ?
                std::unique_ptr<comparison::results>(std::move(m_results_largestBlock)) :
                std::unique_ptr<comparison::results>(nullptr);
}

std::unique_ptr<offsetMetrics::results> comparisonThread::getResults_sequential()
{
    QMutexLocker lock(&m_mutex);

    return isFinished() ?
                std::unique_ptr<offsetMetrics::results>(std::move(m_results_sequential)) :
                std::unique_ptr<offsetMetrics::results>(nullptr);
}

void comparisonThread::setDataSet1(QSharedPointer<dataSet> dataSet1)
{
    QMutexLocker lock(&m_mutex);
    m_dataSet1 = dataSet1;
}

void comparisonThread::setDataSet2(QSharedPointer<dataSet> dataSet2)
{
    QMutexLocker lock(&m_mutex);
    m_dataSet2 = dataSet2;
}

void comparisonThread::run()
{
    QMutexLocker lock(&m_mutex);
    m_results_largestBlock = nullptr;
    m_results_sequential = nullptr;

    if (!m_dataSet1 || !m_dataSet2) {
        return;
    }

    const dataSet::DataReadLock& DRL1 = m_dataSet1->getReadLock();
    const std::vector<unsigned char> &dS1 = DRL1.getData();

    const dataSet::DataReadLock& DRL2 = m_dataSet2->getReadLock();
    const std::vector<unsigned char> &dS2 = DRL2.getData();

    switch (m_comparisonAlgorithm) {

        case comparisonAlgorithm::largestBlock:
            m_results_largestBlock = comparison::doCompare(dS1, dS2);
            break;

        case comparisonAlgorithm::sequential:
            m_results_sequential = offsetMetrics::doCompare(dS1, dS2);
            break;

        default:
            FAIL();
    }
}
