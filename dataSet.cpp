#include "dataSet.h"

dataSet::dataSet() :
    m_data(new QVector<unsigned char>()),
    m_fileName(new QString())
{
    m_loaded = false;
    emit sizeChanged(0);
}

QVector<unsigned char>* dataSet::getData()
{
    return m_data.data();
}

bool dataSet::loadFile(QString fileName)
{
    m_loaded = false;
    m_fileName->clear();

    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDataStream in(&file);

    int s = file.size();
    char *rawFile = new char[s];
    in.readRawData(rawFile, s);

    m_data->clear();
    m_data->resize(s);

    qCopy(rawFile, rawFile+s, m_data->begin());
    file.close();

    delete[] rawFile;

    emit sizeChanged(m_data->size());

    m_loaded = true;
    *(m_fileName.data()) = fileName;
    return true;
}

bool dataSet::compare(dataSet& dataSet1, dataSet& dataSet2, QVector<byterange>& diffs)
{
    if (dataSet1.m_data->size() != dataSet2.m_data->size()){
        return false;
    }

    QVector<unsigned char>::iterator it_dataset1 = dataSet1.m_data->begin();
    QVector<unsigned char>::iterator it_dataset2 = dataSet2.m_data->begin();
    bool openrange = false;
    int byteindex;

    while (it_dataset1 != dataSet1.m_data->end() && it_dataset2 != dataSet2.m_data->end()) {
        if (*it_dataset1 != *it_dataset2){

            byteindex = it_dataset1 - dataSet1.m_data->begin();

            if (openrange){
                byterange& last = diffs.back();
                last.count++;
            } else {
                diffs.push_back(byterange(byteindex));
                openrange = true;
            }

            std::cout << byteindex << ": " << int(*it_dataset1) << ", " << int(*it_dataset2) << std::endl;
        } else {
            openrange = false;
        }

        it_dataset1++;
        it_dataset2++;
    }

    return true;
}
