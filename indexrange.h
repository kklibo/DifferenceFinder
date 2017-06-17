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

    indexRange(){
        start = 0;
        end = 0;
    }

    indexRange(unsigned int) = delete;
    // intentionally not implemented to prevent unintended type conversion
    /* removed to prevent unintended type conversion

    //creates a range of 1 byte at the start index
    byteRange(unsigned int start){
        this->start = start;
        count = 1;
    }*/

    indexRange(unsigned int start, unsigned int end){
        this->start = start;
        this->end = end;
    }

    //returns the number of indices in this indexRange
    unsigned int count() const {
        if (end <= start) {
            return 0;
        }

        return end - start;
    }

    bool contains(const unsigned int& index) const {
        return (    ( start <= index )
                 && ( index <  end   )  );
    }

    bool overlaps(const indexRange& r) const {
        return 0 < getIntersection(r).count();
    }



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

    bool operator==(const indexRange &r) const {
        if ( (0 == count()) && (0 == r.count())) {
            return true;
        }

        return (start == r.start) && (end == r.end);
    }
    bool operator!=(const indexRange &r) const {
        return !( *this == r );
    }
    bool operator< (const indexRange &r) const {
        return start < r.start;
    }

    indexRange getIntersection(const indexRange& r) const {

        unsigned int start  = std::max(this->start, r.start);
        unsigned int end    = std::min(this->end,   r.end);

        if (end <= start) {
            return indexRange(0,0);  //no intersection, return size zero indexRange
        }

        return indexRange(start, end);
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
    static void fillEmptySpaces(const indexRange& fillThisRange, std::list<indexRange>& blocks, std::list<indexRange>& copiesOfAddedBlocks)
    {
        ASSERT(isNonDecreasingAndNonOverlapping(blocks));

        unsigned int currentGapStart = fillThisRange.start;

        for (std::list<indexRange>::iterator it = blocks.begin(); it != blocks.end(); ++it) {

            if (0 == it->count()) {
                //skip blocks w/ count 0 (don't break filler blocks on them)
                continue;
            }

            const unsigned int gapEnd = std::min( it->start, fillThisRange.end );

            if (currentGapStart < gapEnd) { //if there's a gap

                //make a block to fill the current gap and record it
                indexRange newFiller(currentGapStart, gapEnd);
                blocks              .insert(it, newFiller);
                copiesOfAddedBlocks .push_back(newFiller);
            }

            if (gapEnd == fillThisRange.end) {
                //the end of the range has been reached
                return;
            }

            //continue search at first index past this block
            currentGapStart = it->end;
        }

        if (currentGapStart < fillThisRange.end) {
            //make a block to fill the rest of the range and record it
            indexRange lastBlock(currentGapStart, fillThisRange.end);
            blocks              .push_back(lastBlock);
            copiesOfAddedBlocks .push_back(lastBlock);
        }
    }

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
    static bool isExactAscendingPartition(const indexRange& partitionRange, const std::list<indexRange>& blocks)
    {
        if (    0 == partitionRange.count() ||
                0 == blocks.size()  )
        {
            return false;
        }

        unsigned int nextRequiredIndex

            //for an exact ascending partition, the first block should have the same start index as partitionRange
            = partitionRange.start;

        for (auto it = blocks.begin(); it != blocks.end(); ++it)
        {
            //if this block doesn't start where it was required to, it isn't an exact ascending partition
            if (nextRequiredIndex != it->start) {
                return false;
            }

            //indexRange::end is 1 index past the last index in the range
            //  if this is an exact ascending partition, the next block must start there
            nextRequiredIndex = it->end;
        }

        //if this is reached: the blocks form an exact ascending partition of partitionRange
        //  if and only if the last block ends exactly at the end of partitionRange
        return (nextRequiredIndex == partitionRange.end);
    }

    void move(unsigned int newStart)
    {
        unsigned int c = count();
        start = newStart;

        ASSERT(noSumOverflow(start,c));
        end = start+c;
    }
};

#endif // INDEXRANGE_H
