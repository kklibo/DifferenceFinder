#ifndef DATASET_H
#define DATASET_H

#include <QVector>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QMutex>
#include <vector>


#include "log.h"
#include "byteRange.h"
#include "defensivecoding.h"

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
        SUCCESS,
        ERROR_ActiveDataReadLock,
        ERROR_FileDoesNotExist,
        ERROR_FileReadFailure
    };
    loadFileResult loadFile(const QString fileName);

    //index-to-index comparison between two files, outputs differences in diffs
    enum class compareResult {
        SUCCESS,
        ERROR_SizeMismatch
    };
    static compareResult compare(const dataSet& dataSet1, const dataSet& dataSet2, QVector<byteRange>& diffs);


        //DataReadLock is used to control access to m_data: all access outside of this class is done by acquiring
        //  a DataReadLock object.
        class DataReadLock {
            friend class dataSet;
        private:
            explicit DataReadLock(unsigned int& dataReadLockCount, QMutex& mutex, const std::vector<unsigned char>& data)
                :   ref_dataReadLockCount(dataReadLockCount),
                    ref_mutex(mutex),
                    ref_data(data)
            {
                //no mutex lock here: this constructor is only called inside a mutex lock in dataSet::getReadLock
                ++ref_dataReadLockCount;
            }

        public:
            DataReadLock()                          = delete;
            DataReadLock(const DataReadLock& )      = delete;
            DataReadLock(      DataReadLock&&) = default;   //needed for dataSet::getReadLock to return a DataReadLock
            DataReadLock& operator=(DataReadLock& ) = delete;
            DataReadLock& operator=(DataReadLock&&) = delete;

            ~DataReadLock()
            {
                QMutexLocker lock(&ref_mutex);
                --ref_dataReadLockCount;
            }

            const std::vector<unsigned char>& getData() const {
                return ref_data;
            }

        private:
            //this class doesn't "own" any of its own objects:
            //these members are all references to the corresponding members in the dataSet being locked by this DataReadLock
            unsigned int                        & ref_dataReadLockCount;    //the dataSet's active DataReadLock object count
            QMutex                              & ref_mutex;                //the dataSet's mutex
            const std::vector<unsigned char>    & ref_data;                 //the dataSet's byte data
        };



    const DataReadLock getReadLock() const;
    unsigned int getSize() const;
    bool isLoaded() const;
    const QString getFileName() const;

private:
    mutable QMutex m_mutex;
    QScopedPointer<std::vector<unsigned char>> m_data;
    QScopedPointer<QString> m_fileName;
    bool m_loaded;

    //active DataReadLock instances count
    mutable unsigned int m_dataReadLockCount;
    // Thread-safe multiple read-only access to a dataSet can be done by creating multiple DataReadLock instances.
    //
    // Changes to m_dataReadLockCount happen when DataReadLock instances are created or destroyed,
    //  and are always done while m_mutex is locked.
    //
    // DataReadLock::getData calls do not lock m_mutex (so that multiple concurrent uses are possible).
    //  To preserve thread safety, a dataSet should only be modified when m_dataReadLockCount is zero:
    //  otherwise, it could modify data that is being used by another thread that has an active DataReadLock.

};

#endif // DATASET_H
