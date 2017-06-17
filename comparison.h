#ifndef COMPARISON_H
#define COMPARISON_H

#include <vector>
#include <queue>
#include <set>
#include <memory>
#include <utility>
#include <atomic>
#include "blockmatchset.h"
#include "indexrange.h"
#include "buzhash.h"

#include "defensivecoding.h"
#include "stopwatch.h"


#include <iostream>

class comparison
{
public:

    comparison() = delete;  //static functions only

    enum class whichDataSet{
        first,
        second
    };

    class results {
    public:
        std::multiset<blockMatchSet> matches;
        std::list<indexRange> data1_unmatchedBlocks;
        std::list<indexRange> data2_unmatchedBlocks;

        bool aborted;
        bool internalError;

        results() : aborted(false), internalError(false) {}
    };




    static unsigned int findLargestMatchingBlocks(  const std::vector<unsigned char>&   data1,
                                                    const std::vector<unsigned char>&   data2,
                                                    const std::multiset<indexRange>&    data1SkipRanges,
                                                    const std::multiset<indexRange>&    data2SkipRanges,
                                                          std::multiset<blockMatchSet>& matches );

    static bool blockMatchSearch(   const unsigned int blockLength,
                                    const std::vector<unsigned char>& data1,
                                    const std::vector<unsigned char>& data2,
                                    const std::multiset<indexRange>& data1SkipRanges,
                                    const std::multiset<indexRange>& data2SkipRanges,
                                          std::multiset<blockMatchSet>* allMatches = nullptr );

    static void chooseValidMatchSet(       blockMatchSet& match,
                                     const std::vector<unsigned int>& alreadyChosen1,
                                     const std::vector<unsigned int>& alreadyChosen2 );

    static void chooseValidMatchSets( std::multiset<blockMatchSet>& matches );

    static void addMatchesToSkipRanges(   const std::multiset<blockMatchSet>& matches,
                                                std::multiset<indexRange>&    data1SkipRanges,
                                                std::multiset<indexRange>&    data2SkipRanges );

    static std::unique_ptr<std::list<indexRange>> findUnmatchedBlocks(  const indexRange& fillThisRange,
                                                                        const std::multiset<blockMatchSet>& matches,
                                                                        const whichDataSet which );

    static std::unique_ptr<comparison::results> doCompare(  const std::vector<unsigned char>& data1,
                                                            const std::vector<unsigned char>& data2 );

    static void abort();

private:
    static std::atomic_bool m_abort;  //abort flag

};

#endif // COMPARISON_H
