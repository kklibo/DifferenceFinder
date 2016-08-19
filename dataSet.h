#ifndef DATASET_H
#define DATASET_H

#include <QVector>
#include <QString>

#include <iostream>

//#include <QFileDialog>
#include <QFile>
//#include <QMessageBox>
//#include <QTextStream>
#include <QDataStream>

#include "bincomp.h"


class dataSet : public QObject
{
    Q_OBJECT

public:
    dataSet();
    bool loadFile(QString fileName);
    //static bool compare(QVector<unsigned char> dataset1, QVector<unsigned char> dataset2, QVector<byterange>& diffs);
    static bool compare(dataSet& dataSet1, dataSet& dataSet2, QVector<byterange>& diffs);

    QVector<unsigned char>* getData();
    //unsigned char operator [] (int i) const;

signals:
    void sizeChanged(int newSize);

private:
    QScopedPointer<QVector<unsigned char>> m_data;
    QScopedPointer<QString> m_fileName;
    bool m_loaded;

};

#endif // DATASET_H
