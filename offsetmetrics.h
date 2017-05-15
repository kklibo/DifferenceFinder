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

    class results {
    public:
        std::list<byteRange> file1_matches;
        std::list<byteRange> file1_differences;
        std::list<byteRange> file2_matches;
        std::list<byteRange> file2_differences;

        bool aborted;
        bool internalError;

        results() : aborted(false), internalError(false) {}
    };




    static std::unique_ptr<rangeMatch> getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                                const std::vector<unsigned char>& target,
                                                                const byteRange sourceSearchRange,
                                                                const byteRange targetSearchRange
                                                                );

    static std::unique_ptr<rangeMatch> getNextAlignmentRange( const std::vector<unsigned char>& source,
                                                              const std::vector<unsigned char>& target,
                                                              const byteRange sourceSearchRange,
                                                              //this should be sorted by increasing start index
                                                              const std::list<byteRange>& targetSearchRanges
                                                              );

    static void getAlignmentRangeDiff(  const std::vector<unsigned char>& file1,
                                        const std::vector<unsigned char>& file2,
                                        const rangeMatch& alignmentRange,
                                        std::list<byteRange>& file1_matches,
                                        std::list<byteRange>& file1_differences,
                                        std::list<byteRange>& file2_matches,
                                        std::list<byteRange>& file2_differences
                                        );

    static
    std::unique_ptr<offsetMetrics::results>
    doCompare(  const std::vector<unsigned char>& data1,
                const std::vector<unsigned char>& data2 );

    static void abort();

private:
    static std::atomic_bool m_abort;  //abort flag

};

#endif // OFFSETMETRICS_H
