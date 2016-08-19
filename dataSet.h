#ifndef DATASET_H
#define DATASET_H

#include <QVector>
#include <QString>

#include <iostream>

#include <QFile>
#include <QDataStream>

#include "bincomp.h"


class dataSet : public QObject
{
    Q_OBJECT

public:
    dataSet();
    bool loadFile(QString fileName);
    static bool compare(dataSet& dataSet1, dataSet& dataSet2, QVector<byterange>& diffs);

    QVector<unsigned char>* getData();

signals:
    void sizeChanged(int newSize);

private:
    QScopedPointer<QVector<unsigned char>> m_data;
    QScopedPointer<QString> m_fileName;
    bool m_loaded;

};

#endif // DATASET_H
