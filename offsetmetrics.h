#ifndef OFFSETMETRICS_H
#define OFFSETMETRICS_H

#include <vector>
#include <memory>
#include <limits>
#include <list>

#include "byteRange.h"
#include "rangematch.h"
#include "defensivecoding.h"

/*
    Offset Metrics -based comparison
    The general idea:
        starting at the beginning of a file, for each byte (b):
            starting at the beginning of the other file,
            find the index offset to the next occurrence of (b) (continuing beyond already-matched indices)
        after this file to file offset map is generated, analyze offsets and look for groups
*/




class offsetMetrics
{
public:
/*
    class alignmentRange {
    public:
        byteRange rangeInFile1;
        int offset_File1ToFile2;

        alignmentRange(byteRange _rangeInFile1, int _offset_File1ToFile2)
            :   rangeInFile1        (_rangeInFile1),
                offset_File1ToFile2 (_offset_File1ToFile2) {}


    };
*/


    offsetMetrics();


    static std::unique_ptr<std::vector<int>> getOffsetMap(  const std::vector<unsigned char>& source,
                                                            const std::vector<unsigned char>& target,
                                                            const unsigned int sourceStartIndex,
                                                            const unsigned int targetStartIndex
                                                            );

    static std::unique_ptr<rangeMatch> getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                                const std::vector<unsigned char>& target,
                                                                const unsigned int sourceStartIndex,
                                                                const unsigned int targetStartIndex
                                                                );

    static void getAlignmentRangeDiff(  const std::vector<unsigned char>& file1,
                                        const std::vector<unsigned char>& file2,
                                        const rangeMatch& alignmentRange,
                                        std::list<byteRange>& file1_matches,
                                        std::list<byteRange>& file1_differences,
                                        std::list<byteRange>& file2_matches,
                                        std::list<byteRange>& file2_differences
                                        );

};

#endif // OFFSETMETRICS_H
