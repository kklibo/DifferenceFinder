#ifndef INDEXRANGE_H
#define INDEXRANGE_H

#include <list>
#include <algorithm>

#include "defensivecoding.h"

/*
  [based on / replacement for byteRange.h]

  specifies a contiguous group of indices
  any index >= start AND < end is included

  indexRanges with zero included indices are considered equal,
  even if their start and end values are different (they both represent the empty set)

*/

class indexRange {
public:
    unsigned int start; //the inclusive lower bound of the range
    unsigned int end;   //the exclusive upper bound of the range

    indexRange();
    indexRange(unsigned int) = delete;  // intentionally not implemented to prevent unintended type conversion
    indexRange(unsigned int start, unsigned int end);

    void move(unsigned int newStart);

    bool operator==(const indexRange &r) const;
    bool operator!=(const indexRange &r) const;
    bool operator< (const indexRange &r) const; //compares start index only

    unsigned int    count()                                 const; //the number of included indices
    bool            contains(const unsigned int& index)     const; //true iff index is contained in this indexRange
    bool            overlaps(const indexRange& r)           const; //true iff this has an index in common with r
    indexRange      getIntersection(const indexRange& r)    const; //all indices included in this and r


    //for containers of indexRanges
    template <typename T>
    bool overlapsAnyIn(const T& indexRanges) const {
        for (const indexRange& r : indexRanges) {
            if (overlaps(r)) {
                return true;
            }
        }
        return false;
    }

    //for containers of start indices (with a fixed count value)
    template <typename T>
    bool overlapsAnyIn(const T& startIndices, const unsigned int& count) const {
        for (const unsigned int& r : startIndices) {

            ASSERT(noSumOverflow(r,count));
            //overflows could be truncated to unsigned int max

            if (overlaps( indexRange(r, r + count) )) {
                return true;
            }
        }
        return false;
    }


    template <typename T>
    static bool isNonDecreasingAndNonOverlapping(const T& indexRanges) {

        unsigned int minStart = 0;  //minimum start for subsequent acceptible indexRanges

        for (const indexRange& r : indexRanges) {

            if (0 == r.count()) {
                continue;   //skip empty indexRanges
            }

            if (r.start < minStart) {
                return false;   //the start of this range is before or inside a previous range
            }

            minStart = r.end; //update minStart and continue stepping through the indexRanges

        }

        return true;    //no violations found
    }

    template <typename T>
    static bool isNonDecreasingAndNonOverlapping(const T& startIndices, const unsigned int& count) {

        unsigned int minStart = 0;  //minimum start for subsequent acceptible byteRanges

        for (const unsigned int& i : startIndices) {

            if (i < minStart) {
                return false;   //the start of this range is before or inside a previous range
            }

            ASSERT(noSumOverflow(i,count));
            minStart = i + count; //update minStart and continue stepping through the byteRanges

        }

        return true;    //no violations found
    }

    template <typename T>
    static bool isNonDecreasing(const T& startIndices) {

        unsigned int minStart = 0;  //minimum start for subsequent acceptible start indices

        for (const unsigned int& i : startIndices) {

            if (i < minStart) {
                return false;   //the start of this range is before or inside a previous range
            }

            minStart = i; //update minStart and continue stepping through the indices

        }

        return true;    //no violations found
    }

    //if possible, call isNonDecreasingAndNonOverlapping instead:
    // this function copies and sorts the byteRanges
    //note: this function throws out byteRanges with count 0 (they can't overlap anything)
    template <typename T>
    static bool isNonOverlapping(const T& indexRanges) {

        std::vector<indexRange> sortCopy;
        for (indexRange r : indexRanges) {
            if (0 < r.count()) {
                sortCopy.push_back(r);
            }
        }
        std::sort(sortCopy.begin(), sortCopy.end());

        return isNonDecreasingAndNonOverlapping(sortCopy);
    }

    //if possible, call isNonDecreasingAndNonOverlapping instead:
    // this function copies and sorts the indices
    template <typename T>
    static bool isNonOverlapping(const T& startIndices, const unsigned int& count) {

        if ( 0 == count ) {
            return true;    //count 0 ranges don't overlap anything
        }

        std::vector<indexRange> sortCopy;
        for (const unsigned int& i : startIndices) {

            ASSERT(noSumOverflow(i,count));
            sortCopy.push_back(indexRange(i, i + count));
        }
        std::sort(sortCopy.begin(), sortCopy.end());

        return isNonDecreasingAndNonOverlapping(sortCopy);
    }

    //this function assumes isNonDecreasingAndNonOverlapping(blocks) would return true
    static void fillEmptySpaces(const indexRange& fillThisRange, std::list<indexRange>& blocks, std::list<indexRange>& copiesOfAddedBlocks);

    //this function assumes isNonDecreasingAndNonOverlapping(aroundTheseBlocks) would return true
    template <typename T>
    static unsigned int findSizeOfLargestEmptySpace(const indexRange& inThisRange, const T& aroundTheseBlocks)
    {
        ASSERT(isNonDecreasingAndNonOverlapping(aroundTheseBlocks));

        unsigned int largestGapSize = 0;

        unsigned int currentGapStart = inThisRange.start;

        for (auto it = aroundTheseBlocks.begin(); it != aroundTheseBlocks.end(); ++it) {

            if (0 == it->count()) {
                //skip blocks w/ count 0 (don't break filler blocks on them)
                continue;
            }

            const unsigned int gapEnd = std::min( it->start, inThisRange.end );

            if (currentGapStart < gapEnd) { //if there's a gap

                //update largestGapSize if this gap is larger
                unsigned int gapSize = gapEnd - currentGapStart;
                largestGapSize = std::max(largestGapSize, gapSize);
            }

            if (gapEnd == inThisRange.end) {
                //the end of the range has been reached
                return largestGapSize;
            }

            //continue search at first index past this block
            currentGapStart = it->end;
        }

        if (currentGapStart < inThisRange.end) {
            //check the gap between the last block and the end of the search range
            unsigned int gapSize = inThisRange.end - currentGapStart;
            largestGapSize = std::max(largestGapSize, gapSize);
        }

        return largestGapSize;
    }

    //
    //  returns true if and only if blocks
    //      include every index in partitionRange exactly once
    //          (i.e., no gaps between blocks, and no overlapping blocks),
    //      include no indices outside partitionRange,
    //      and are in ascending order
    //
    //  note: returns false for size 0 partition range or an empty list of blocks,
    //      because these states are expected to be errors
    //
    static bool isExactAscendingPartition(const indexRange& partitionRange, const std::list<indexRange>& blocks);

};

#endif // INDEXRANGE_H
