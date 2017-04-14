#ifndef BYTERANGE_H
#define BYTERANGE_H

#include <vector>
#include <list>
#include <algorithm>

/*
    specifies a range of bytes
*/

class byteRange {
public:
    unsigned int start; //the first index in the range
    unsigned int count; //the number of bytes

    byteRange(){
        start = 0;
        count = 0;
    }

    byteRange(unsigned int) = delete;
    /* removed to prevent unintended type conversion

    //creates a range of 1 byte at the start index
    byteRange(unsigned int start){
        this->start = start;
        count = 1;
    }*/

    byteRange(unsigned int start, unsigned int count){
        this->start = start;
        this->count = count;
    }

    //returns the index that is one past the end of this byteRange
    unsigned int end() const {
        return start + count;
    }

    bool overlaps(const byteRange& b) const {

        if ((0 == count) || (0 == b.count)) {
            return false;   //ranges w/ count 0 don't overlap anything
        }

        if(start < b.start) {
            return b.start < end();
        }
        else {
            return start < b.end();
        }
    }

    //for containers of byteRanges
    template <typename T>
    bool overlapsAnyIn(const T& byteRanges) const {
        for (const byteRange& r : byteRanges) {
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
            if (overlaps( byteRange(r,count) )) {
                return true;
            }
        }
        return false;
    }

    bool operator==(const byteRange& b) const {
        return (start == b.start) && (count == b.count);
    }
    bool operator!=(const byteRange& b) const {
        return !( *this == b );
    }
    bool operator< (const byteRange& b) const {
        return start < b.start;
    }

    //note: a byterange repeating the start index of a prior byteRange w/count zero
    //  will not cause this to return false (this shouldn't come up in normal use)
    template <typename T>
    static bool isNonDecreasingAndNonOverlapping(const T& byteRanges) {

        unsigned int minStart = 0;  //minimum start for subsequent acceptible byteRanges

        for (const byteRange& r : byteRanges) {

            if (r.start < minStart) {
                return false;   //the start of this range is before or inside a previous range
            }

            minStart = r.end(); //update minStart and continue stepping through the byteRanges

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
    static bool isNonOverlapping(const T& byteRanges) {

        std::vector<byteRange> sortCopy;
        for (byteRange b : byteRanges) {
            if (0 < b.count) {
                sortCopy.push_back(b);
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

        std::vector<byteRange> sortCopy;
        for (const unsigned int& i : startIndices) {
            sortCopy.push_back(byteRange(i, count));
        }
        std::sort(sortCopy.begin(), sortCopy.end());

        return isNonDecreasingAndNonOverlapping(sortCopy);
    }

    //this function assumes isNonDecreasingAndNonOverlapping(blocks) would return true
    static void fillEmptySpaces(const byteRange& fillThisRange, std::list<byteRange>& blocks, std::list<byteRange>& copiesOfAddedBlocks)
    {
        unsigned int currentGapStart = fillThisRange.start;

        for (std::list<byteRange>::iterator it = blocks.begin(); it != blocks.end(); ++it) {

            if (0 == it->count) {
                //skip blocks w/ count 0 (don't break filler blocks on them)
                continue;
            }

            const unsigned int gapEnd = std::min( it->start, fillThisRange.end() );

            if (currentGapStart < gapEnd) { //if there's a gap

                //make a block to fill the current gap and record it
                byteRange newFiller(currentGapStart, gapEnd - currentGapStart);
                blocks              .insert(it, newFiller);
                copiesOfAddedBlocks .push_back(newFiller);
            }

            if (gapEnd == fillThisRange.end()) {
                //the end of the range has been reached
                return;
            }

            //continue search at first index past this block
            currentGapStart = it->end();
        }

        if (currentGapStart < fillThisRange.end()) {
            //make a block to fill the rest of the range and record it
            byteRange lastBlock(currentGapStart, fillThisRange.end() - currentGapStart);
            blocks              .push_back(lastBlock);
            copiesOfAddedBlocks .push_back(lastBlock);
        }
    }

    //this function assumes isNonDecreasingAndNonOverlapping(blocks) would return true
    template <typename T>
    static unsigned int findSizeOfLargestEmptySpace(const byteRange& inThisRange, const T& aroundTheseBlocks)
    {
        unsigned int largestGapSize = 0;

        unsigned int currentGapStart = inThisRange.start;

        for (auto it = aroundTheseBlocks.begin(); it != aroundTheseBlocks.end(); ++it) {

            if (0 == it->count) {
                //skip blocks w/ count 0 (don't break filler blocks on them)
                continue;
            }

            const unsigned int gapEnd = std::min( it->start, inThisRange.end() );

            if (currentGapStart < gapEnd) { //if there's a gap

                //update largestGapSize if this gap is larger
                unsigned int gapSize = gapEnd - currentGapStart;
                largestGapSize = std::max(largestGapSize, gapSize);
            }

            if (gapEnd == inThisRange.end()) {
                //the end of the range has been reached
                return largestGapSize;
            }

            //continue search at first index past this block
            currentGapStart = it->end();
        }

        if (currentGapStart < inThisRange.end()) {
            //check the gap between the last block and the end of the search range
            unsigned int gapSize = inThisRange.end() - currentGapStart;
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
    static bool isExactAscendingPartition(const byteRange& partitionRange, const std::list<byteRange>& blocks)
    {
        if (    0 == partitionRange.count ||
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

            //byteRange::end() returns 1 index past the last index in the range
            //  if this is an exact ascending partition, the next block must start there
            nextRequiredIndex = it->end();
        }

        //if this is reached: the blocks form an exact ascending partition of partitionRange
        //  if and only if the last block ends exactly at the end of partitionRange
        return (nextRequiredIndex == partitionRange.end());
    }

};

#endif // BYTERANGE_H
