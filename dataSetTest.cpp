#include <QSharedPointer>
#include <QVector>

#include <QString>
#include <QStringBuilder>

#include "dataSet.h"
#include "byteRange.h"

#include "gtestDefs.h"

//  gtest.h and defensivecoding.h have macro conflicts (FAIL and ASSERT_EQ so far)
//  this is solved (for now) by including gtest.h last
#include <gtest.h>

TEST(dataSet, LoadBadFileNames){
    dataSet dataSet1;
    dataSet::loadFileResult res1;
    res1 = dataSet1.loadFile("");
    EXPECT_EQ(dataSet::loadFileResult::ERROR_FileDoesNotExist, res1) << "empty file name handled incorrectly";
    res1 = dataSet1.loadFile("\0");
    EXPECT_EQ(dataSet::loadFileResult::ERROR_FileDoesNotExist, res1) << "invalid file name handled incorrectly";
    res1 = dataSet1.loadFile("thisfiledoesnotexist");
    EXPECT_EQ(dataSet::loadFileResult::ERROR_FileDoesNotExist, res1) << "non-existent file name handled incorrectly";
}

TEST(dataSet, LoadAndCompareSameFile){
    dataSet dataSet1;
    dataSet dataSet2;

    dataSet::loadFileResult res1, res2;
    res1 = dataSet1.loadFile(gtestDefs::testFilePath % "test2_1");
    res2 = dataSet2.loadFile(gtestDefs::testFilePath % "test2_1");
    EXPECT_EQ(dataSet::loadFileResult::SUCCESS, res1) << "dataSet1 load failed";
    EXPECT_EQ(dataSet::loadFileResult::SUCCESS, res2) << "dataSet2 load failed";

    QVector<byteRange> diffs;
    dataSet::compare(dataSet1, dataSet2, diffs);

    EXPECT_EQ(0, diffs.length()) << "nonzero difference count when comparing a file to itself";
}

TEST(dataSet, LoadAndCompareSameSizeFiles){
    dataSet dataSet1;
    dataSet dataSet2;

    dataSet::loadFileResult res1, res2;
    res1 = dataSet1.loadFile(gtestDefs::testFilePath % "LoadAndCompareSameSizeFiles1");
    res2 = dataSet2.loadFile(gtestDefs::testFilePath % "LoadAndCompareSameSizeFiles2");
    EXPECT_EQ(dataSet::loadFileResult::SUCCESS, res1) << "dataSet1 load failed";
    EXPECT_EQ(dataSet::loadFileResult::SUCCESS, res2) << "dataSet2 load failed";

    QVector<byteRange> expected = {byteRange(5,1)};
    QVector<byteRange> diffs = QVector<byteRange>();
    dataSet::compare(dataSet1, dataSet2, diffs);

    EXPECT_EQ(expected, diffs) << "wrong diffs detected";
}
