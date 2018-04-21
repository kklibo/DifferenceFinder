#ifndef OFFSETMETRICS_H
#define OFFSETMETRICS_H

#include <vector>
#include <memory>
#include <limits>
#include <list>
#include <atomic>

#include "indexrange.h"
#include "rangematch.h"
#include "utilities.h"
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
        std::list<indexRange> file1_matches;
        std::list<indexRange> file1_differences;
        std::list<indexRange> file2_matches;
        std::list<indexRange> file2_differences;

        bool aborted;
        bool internalError;

        results() : aborted(false), internalError(false) {}
    };




    static std::unique_ptr<rangeMatch> getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                                const std::vector<unsigned char>& target,
                                                                const unsigned int sourceRangeStart,
                                                                const indexRange sourceSearchRange,
                                                                const indexRange targetSearchRange
                                                                );

    static std::unique_ptr<rangeMatch> getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                                const std::vector<unsigned char>& target,
                                                                const indexRange sourceSearchRange,
                                                                const indexRange targetSearchRange
                                                                );

    static std::unique_ptr<rangeMatch> getNextAlignmentRange( const std::vector<unsigned char>& source,
                                                              const std::vector<unsigned char>& target,
                                                              const indexRange sourceSearchRange,
                                                              //this should be sorted by increasing start index
                                                              const std::list<indexRange>& targetSearchRanges
                                                              );

    static bool isNonMatchRangeExcludable(  const std::vector<unsigned char>& source,
                                            const std::vector<unsigned char>& target,
                                            const indexRange sourceNonMatchRange,
                                            const indexRange targetNonMatchRange
                                            );

    static bool truncateAlignmentRange( const std::vector<unsigned char>& data1,
                                        const std::vector<unsigned char>& data2,
                                              rangeMatch& alignmentRange
                                        );

    static void getAlignmentRangeDiff(  const std::vector<unsigned char>& file1,
                                        const std::vector<unsigned char>& file2,
                                        const rangeMatch& alignmentRange,
                                        std::list<indexRange>& file1_matches,
                                        std::list<indexRange>& file1_differences,
                                        std::list<indexRange>& file2_matches,
                                        std::list<indexRange>& file2_differences
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
