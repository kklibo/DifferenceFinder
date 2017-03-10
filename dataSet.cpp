#include "dataSet.h"

dataSet::dataSet() :
    m_data(new QVector<unsigned char>()),
    m_fileName(new QString())
{
    m_loaded = false;
}

//returns a raw pointer to the data, is there a safer way?
QVector<unsigned char>* dataSet::getData() const
{
    return m_data.data();
}

unsigned int dataSet::getSize() const
{
    if (!m_loaded) {
        return 0;
    }

    int s = m_data->size();
    ASSERT_NOT_NEGATIVE(s);
    return static_cast<unsigned int>(s);
}

bool dataSet::isLoaded() const
{
    return m_loaded;
}

dataSet::loadFileResult dataSet::loadFile(const QString fileName)
{
    this->reset();

    QFile file(fileName);   //QFile will close itself when it is released
    if (!file.open(QIODevice::ReadOnly)) {
        return loadFileResult::ERROR_FileDoesNotExist;
    }

    QDataStream in(&file);

    //get filesize, constrain to int for QDataStream::readRawData
    qint64 s = file.size();
    if (s > INT32_MAX) {
        return loadFileResult::ERROR_FileReadFailure;
    }
    int fileSize = static_cast<int>(s);

    QScopedArrayPointer<char> rawFile(new char[fileSize]);
    if (-1 == in.readRawData(rawFile.data(), fileSize))
    {
        return loadFileResult::ERROR_FileReadFailure;
    }

    m_data->resize(fileSize);

    std::copy(rawFile.data(), rawFile.data()+fileSize, m_data->begin());

    m_loaded = true;
    Q_ASSERT(Q_NULLPTR != m_fileName.data());
    *(m_fileName.data()) = fileName;

    return loadFileResult::SUCCESS;
}

void dataSet::reset()
{
    m_loaded = false;
    m_fileName->clear();
    m_data->clear();
}

dataSet::compareResult dataSet::compare(const dataSet& dataSet1, const dataSet& dataSet2, QVector<byteRange>& diffs)
{
    if (dataSet1.m_data->size() != dataSet2.m_data->size()){
        return compareResult::ERROR_SizeMismatch;
    }

    QVector<unsigned char>::iterator it_dataset1 = dataSet1.m_data->begin();
    QVector<unsigned char>::iterator it_dataset2 = dataSet2.m_data->begin();
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
