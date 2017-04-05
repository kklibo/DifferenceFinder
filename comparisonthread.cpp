#include "comparisonthread.h"

comparisonThread::comparisonThread(QObject* parent/*= nullptr*/)
  : QThread (parent),
    m_mutex(),
    m_dataSet1(nullptr),
    m_dataSet2(nullptr),
    m_results(nullptr)
{

}

comparisonThread::~comparisonThread()
{
    comparison::abort();
    wait(); //returns when run() is not running
}

void comparisonThread::doCompare()
{
    start();
}

void comparisonThread::abort()
{
    comparison::abort();
}

std::unique_ptr<comparison::results> comparisonThread::getResults()
{
    QMutexLocker lock(&m_mutex);

    return isFinished() ?
                std::unique_ptr<comparison::results>(std::move(m_results)) :
                std::unique_ptr<comparison::results>(nullptr);
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
    m_results = nullptr;

    if (!m_dataSet1 || !m_dataSet2) {
        return;
    }

    if (!m_dataSet1->getData() || !m_dataSet2->getData()) {
        return;
    }

    //convert from qt vector to std::vector (TODO: FIX THIS)
    std::vector<unsigned char> dS1 = m_dataSet1->getData()->toStdVector();
    std::vector<unsigned char> dS2 = m_dataSet2->getData()->toStdVector();

    m_results = comparison::doCompare(dS1, dS2);
}
