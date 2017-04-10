#include "dataSet.h"

dataSet::dataSet() :
    m_mutex(),
    m_data(new std::vector<unsigned char>()),
    m_fileName(new QString()),
    m_loaded(false),
    m_dataReadLockCount(0)
{
}

const dataSet::DataReadLock dataSet::getReadLock() const
{
    QMutexLocker lock(&m_mutex);
    Q_CHECK_PTR(m_data.data());
    return dataSet::DataReadLock(m_dataReadLockCount, m_mutex, *m_data.data());
}

unsigned int dataSet::getSize() const
{
    QMutexLocker lock(&m_mutex);
    if (!m_loaded) {
        return 0;
    }

    unsigned long s = m_data->size();
    ASSERT_LE_UINT_MAX(s);
    return static_cast<unsigned int>(s);
}

bool dataSet::isLoaded() const
{
    QMutexLocker lock(&m_mutex);
    return m_loaded;
}

dataSet::loadFileResult dataSet::loadFile(const QString fileName)
{
    QMutexLocker lock(&m_mutex);
    if (0 < m_dataReadLockCount) {
        //there is an active DataReadLock:
        // the dataSet may be in use
        return loadFileResult::ERROR_ActiveDataReadLock;
    }

    //reset the dataSet
    m_data->clear();
    m_fileName->clear();
    m_loaded = false;
    m_dataReadLockCount = 0;


    QFile file(fileName);   //QFile will close itself when it is released
    if (!file.open(QIODevice::ReadOnly)) {
        return loadFileResult::ERROR_FileDoesNotExist;
    }

    QDataStream in(&file);

    //get filesize, constrain to int for QDataStream::readRawData
    qint64 s = file.size();
    if (s > INT_MAX) {
        return loadFileResult::ERROR_FileReadFailure;
    }
    int fileSize = static_cast<int>(s);

    QScopedArrayPointer<char> rawFile(new char[fileSize]);
    if (-1 == in.readRawData(rawFile.data(), fileSize))
    {
        return loadFileResult::ERROR_FileReadFailure;
    }

    m_data->resize(static_cast<unsigned long>(fileSize));

    std::copy(rawFile.data(), rawFile.data()+fileSize, m_data->begin());

    m_loaded = true;
    Q_ASSERT(Q_NULLPTR != m_fileName.data());
    *(m_fileName.data()) = fileName;

    return loadFileResult::SUCCESS;
}

/*static*/ dataSet::compareResult dataSet::compare(const dataSet& dataSet1, const dataSet& dataSet2, QVector<byteRange>& diffs)
{
    //lock the dataSets
    const dataSet::DataReadLock& DRL1 = dataSet1.getReadLock();
    const dataSet::DataReadLock& DRL2 = dataSet2.getReadLock();

    if (dataSet1.m_data->size() != dataSet2.m_data->size()){
        return compareResult::ERROR_SizeMismatch;
    }

    std::vector<unsigned char>::iterator it_dataset1 = dataSet1.m_data->begin();
    std::vector<unsigned char>::iterator it_dataset2 = dataSet2.m_data->begin();
    bool inDiffSection = false;   //true when byteindex is in a section of byte differences
    unsigned int byteindex;

    while (it_dataset1 != dataSet1.m_data->end() && it_dataset2 != dataSet2.m_data->end()) {
        if (*it_dataset1 != *it_dataset2){

            {
                //get the byte index (from iterator difference)
                long val = it_dataset1 - dataSet1.m_data->begin();
                ASSERT_NOT_NEGATIVE(val);
                ASSERT(val <= UINT_MAX);
                byteindex = static_cast<unsigned int>(val);
            }

            if (inDiffSection){
                Q_ASSERT(0 != diffs.size());
                diffs.last().count++;   //add one to the count of the current diff byteRange
            } else {
                diffs.push_back(byteRange(byteindex, 1));  //add a new diff byteRange of count 1 at this index
                inDiffSection = true;
            }

            QString str;
            QTextStream strStr(&str);
            strStr << "\t" << byteindex << ": " << int(*it_dataset1) << ", " << int(*it_dataset2);
            LOG.Debug(str);


        } else {
            inDiffSection = false;
        }

        it_dataset1++;
        it_dataset2++;
    }

    return compareResult::SUCCESS;
}
