#ifndef COMPARISON_H
#define COMPARISON_H

#include <vector>
#include <queue>
#include <set>
#include <memory>
#include <utility>
#include "rollinghashcpp/cyclichash.h"
#include "rollinghashcpp/rabinkarphash.h"
#include "rollinghashcpp/generalhash.h"
#include "blockmatchset.h"
#include "byteRange.h"

#include "defensivecoding.h"


#include <iostream>

class comparison
{
public:
    comparison();

    static void rollingHashTest();
    void rollingHashTest2();

    unique_ptr<std::vector<unsigned int>> getRollingHashValues(std::vector<unsigned char>& data);

    unsigned int findLargestMatchingBlocks( const std::vector<unsigned char>&   data1,
                                            const std::vector<unsigned char>&   data2,
                                            const std::multiset<byteRange>&     data1SkipRanges,
                                            const std::multiset<byteRange>&     data2SkipRanges,
                                                  std::multiset<blockMatchSet>& matches );

    bool blockMatchSearch(  const unsigned int blockLength,
                            const std::vector<unsigned char>& data1,
                            const std::vector<unsigned char>& data2,
                            const std::multiset<byteRange>& data1SkipRanges,
                            const std::multiset<byteRange>& data2SkipRanges,
                                  std::multiset<blockMatchSet>* allMatches = nullptr );

    static void chooseValidMatchSet( blockMatchSet& match );
    static void chooseValidMatchSets( std::multiset<blockMatchSet>& matches );

    static void addMatchesToSkipRanges(   const std::multiset<blockMatchSet>& matches,
                                                std::multiset<byteRange>&     data1SkipRanges,
                                                std::multiset<byteRange>&     data2SkipRanges );

private:

    void createNewHasher(unsigned int n, unsigned int hashBits);
    unsigned int getNextRollingHashValue(unsigned char nextByte);
    std::shared_ptr<std::queue<unsigned char>> toRemove;
    std::shared_ptr<CyclicHash<>> hasher;

};

#endif // COMPARISON_H
