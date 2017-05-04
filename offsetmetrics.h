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
        use a simple search to find alignment ranges: blocks of >50% matches between the 2 files
        (TODO: then look for alternate related match options that might be better)
*/

class offsetMetrics
{
public:

    offsetMetrics() = delete;   //static functions only

    static std::unique_ptr<rangeMatch> getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                                const std::vector<unsigned char>& target,
                                                                const unsigned int sourceSearchStartIndex,
                                                                const unsigned int targetSearchStartIndex
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
