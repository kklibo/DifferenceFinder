#ifndef DATASET_H
#define DATASET_H

#include <QVector>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QTextStream>

#include "log.h"
#include "byteRange.h"

/*
    Represents the contents of a loaded file as a set of bytes
*/

class dataSet : public QObject
{
    Q_OBJECT

public:
    dataSet();

    //load a file
    enum class loadFileResult {
        SUCCESS, ERROR_FileDoesNotExist, ERROR_FileReadFailure};
    loadFileResult loadFile(const QString fileName);

    //index-to-index comparison between two files, outputs differences in diffs
    enum class compareResult {
        SUCCESS, ERROR_SizeMismatch};
    static compareResult compare(const dataSet& dataSet1, const dataSet& dataSet2, QVector<byteRange>& diffs);

    QVector<unsigned char>* getData() const;

private:
    void reset();   //reset (like fresh instantiation)
    QScopedPointer<QVector<unsigned char>> m_data;
    QScopedPointer<QString> m_fileName;
    bool m_loaded;

};

#endif // DATASET_H
