#include "comparisonthread.h"

comparisonThread::comparisonThread(QObject* parent/*= nullptr*/)
  : QThread (parent),
    m_mutex(),
    m_abort(false),
    m_resultsReady(false),
    m_dataSet1(nullptr),
    m_dataSet2(nullptr)
{

}

comparisonThread::~comparisonThread()
{
    {
        QMutexLocker lock(&m_mutex);
        m_abort = true;
    }

    wait();
}

void comparisonThread::doCompare()
{
    start();
}

QSharedPointer<std::multiset<blockMatchSet>> comparisonThread::getResultMatches()
{
    QMutexLocker lock(&m_mutex);

    return m_resultsReady ?
                QSharedPointer<std::multiset<blockMatchSet>>(m_resultMatches) :
                QSharedPointer<std::multiset<blockMatchSet>>(nullptr);
}

QSharedPointer<std::list<byteRange>> comparisonThread::getData1_unmatchedBlocks()
{
    QMutexLocker lock(&m_mutex);

    return m_resultsReady ?
                QSharedPointer<std::list<byteRange>>(m_result_data1_unmatchedBlocks) :
                QSharedPointer<std::list<byteRange>>(nullptr);
}

QSharedPointer<std::list<byteRange>> comparisonThread::getData2_unmatchedBlocks()
{
    QMutexLocker lock(&m_mutex);

    return m_resultsReady ?
                QSharedPointer<std::list<byteRange>>(m_result_data2_unmatchedBlocks) :
                QSharedPointer<std::list<byteRange>>(nullptr);
}

void comparisonThread::setDataSet1(QSharedPointer<dataSet> dataSet1)
{
    m_dataSet1 = dataSet1;
}

void comparisonThread::setDataSet2(QSharedPointer<dataSet> dataSet2)
{
    m_dataSet2 = dataSet2;
}

void comparisonThread::run()
{
    QMutexLocker lock(&m_mutex);
    m_resultsReady = false;

    comparison C;

    if (!m_dataSet1 || !m_dataSet2) {
        return;
    }

    if (!m_dataSet1->getData() || !m_dataSet2->getData()) {
        return;
    }

    if (!m_dataSet1->getData()->size() || !m_dataSet2->getData()->size()) {
        return;
    }

    std::vector<unsigned char> dS1 = m_dataSet1->getData()->toStdVector();
    std::vector<unsigned char> dS2 = m_dataSet2->getData()->toStdVector();
    std::multiset<byteRange> data1SkipRanges;
    std::multiset<byteRange> data2SkipRanges;
    //std::multiset<blockMatchSet> allMatches;
    m_resultMatches = m_resultMatches.create();

    unsigned int largest = 1;

    do {
        std::multiset<blockMatchSet> matches; //matches for this iteration (will all have the same block size)

        largest = C.findLargestMatchingBlocks(dS1, dS2, data1SkipRanges, data2SkipRanges, matches);
        LOG.Debug(QString("Largest Matching Block Size: %1").arg(largest));

        comparison::addMatchesToSkipRanges(matches, data1SkipRanges, data2SkipRanges);

        if (!byteRange::isNonOverlapping(data1SkipRanges)) {
            LOG.Warning("data1SkipRanges");
        }
        if (!byteRange::isNonOverlapping(data2SkipRanges)) {
            LOG.Warning("data2SkipRanges");
        }

        //add to main match list
        (*m_resultMatches).insert(matches.begin(), matches.end());

    } while (largest > 1);//0);

    byteRange data1_FullRange (0, m_dataSet1->getSize());
    auto data1_unmatchedBlocks = comparison::findUnmatchedBlocks(data1_FullRange, (*m_resultMatches), comparison::whichDataSet::first);

    //release findUnmatchedBlocks results unique_ptr into QSharedPointer
    m_result_data1_unmatchedBlocks.reset(data1_unmatchedBlocks.release());


    byteRange data2_FullRange (0, m_dataSet2->getSize());
    auto data2_unmatchedBlocks = comparison::findUnmatchedBlocks(data2_FullRange, (*m_resultMatches), comparison::whichDataSet::second);

    //release findUnmatchedBlocks results unique_ptr into QSharedPointer
    m_result_data2_unmatchedBlocks.reset(data2_unmatchedBlocks.release());


    m_resultsReady = true;
    emit resultsAreReady();

}

/*
void comparisonThread::run()
{
    int c = 0;
    forever {

        for (unsigned int i = 0; i < static_cast<unsigned int>(-1); ++i) {
            volatile int temp = 0;
        }

        sendMessage(QString("thread loop %1").arg(++c), QColor::fromRgb(192,192,192));

        {
            QMutexLocker lock(&m_mutex);
            if (m_abort) {
                return;
            }
        }
    }
}*/
