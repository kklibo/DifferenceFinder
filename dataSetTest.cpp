#include <gtest.h>

#include "dataSet.h"
#include "QVector"
#include "bincomp.h"


TEST(dataSet, compare1){
    dataSet dataSet1;
    dataSet dataSet2;

    dataSet1.loadFile("test1");
    dataSet2.loadFile("test1");

    QSharedPointer<QVector<byterange>> diffs;
    diffs = QSharedPointer<QVector<byterange>>::create();
    dataSet::compare(dataSet1, dataSet2, *diffs.data());

    int tmp = diffs.data()->length();
    EXPECT_EQ(0, tmp) << "nonzero difference count when comparing a file to itself";
}
