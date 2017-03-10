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

    /*
     * removed to prevent unintended type conversion
     *
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


    //this function assumes isNonDecreasingAndNonOverlapping(blocks) would return true
    static void fillEmptySpaces(const byteRange& fillThisRange, std::list<byteRange>& blocks, std::list<byteRange>& copiesOfAddedBlocks)
    {
        unsigned int currentGapStart = fillThisRange.start;

        for (std::list<byteRange>::iterator it = blocks.begin(); it != blocks.end(); ++it) {

            if (0 == it->count) {
                //skip blocks w/ count 0 (don't break filler blocks on them)
                continue;
            }

            if (currentGapStart < it->start)   //if there's a gap
            {
                //make a block to fill the current gap and record it
                byteRange newFiller(currentGapStart, it->start - currentGapStart);
                blocks              .insert(it, newFiller);
                copiesOfAddedBlocks .push_back(newFiller);
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


    static bool test() {

        bool result = true;
        byteRange a(20, 10);

        result = result && (    a.overlaps(a)                   );

        result = result && (  ! a.overlaps(byteRange(15, 5))    );
        result = result && (    a.overlaps(byteRange(20, 5))    );
        result = result && (    a.overlaps(byteRange(25, 5))    );
        result = result && (  ! a.overlaps(byteRange(30, 5))    );

        result = result && (  ! a.overlaps(byteRange(10,10))    );
        result = result && (    a.overlaps(byteRange(15,10))    );
        result = result && (    a.overlaps(byteRange(20,10))    );
        result = result && (    a.overlaps(byteRange(25,10))    );
        result = result && (  ! a.overlaps(byteRange(30,10))    );

        result = result && (  ! a.overlaps(byteRange( 0,20))    );
        result = result && (    a.overlaps(byteRange( 5,20))    );
        result = result && (    a.overlaps(byteRange(10,20))    );
        result = result && (    a.overlaps(byteRange(15,20))    );
        result = result && (    a.overlaps(byteRange(20,20))    );
        result = result && (    a.overlaps(byteRange(25,20))    );
        result = result && (  ! a.overlaps(byteRange(30,20))    );

        result = result && (  ! a.overlaps(byteRange(15, 0))    );
        result = result && (  ! a.overlaps(byteRange(20, 0))    );
        result = result && (  ! a.overlaps(byteRange(25, 0))    );
        result = result && (  ! a.overlaps(byteRange(30, 0))    );

        byteRange b(20, 0);

        result = result && (  ! b.overlaps(b)                   );
        result = result && (  ! b.overlaps(byteRange(10,10))    );
        result = result && (  ! b.overlaps(byteRange(15,10))    );
        result = result && (  ! b.overlaps(byteRange(20,10))    );

        std::vector<byteRange> r1 = { byteRange(10,10), byteRange(20,10), byteRange(35,0), byteRange(35,10) };
        result = result && (    isNonDecreasingAndNonOverlapping(r1)   );

        std::vector<byteRange> r2 = { byteRange(10,10), byteRange(35,0), byteRange(20,10), byteRange(35,10) };
        result = result && (  ! isNonDecreasingAndNonOverlapping(r2)   );

        std::vector<byteRange> r3 = { byteRange(40,10), byteRange(20,10), byteRange(30,0), byteRange(10,10) };
        result = result && (    isNonOverlapping(r3)   );

        std::vector<byteRange> r4 = { byteRange(40,10), byteRange(20,15), byteRange(30,0), byteRange(10,10) };
        result = result && (    isNonOverlapping(r4)   );

        std::vector<byteRange> r5 = { byteRange(40,10), byteRange(20,15), byteRange(30,1), byteRange(10,10) };
        result = result && (  ! isNonOverlapping(r5)   );

        byteRange c(20,10);
        std::vector<byteRange> o1 = { byteRange(10,10), byteRange(30,10), byteRange(25,0)};
        result = result && (  ! c.overlapsAnyIn(o1)             );
        std::vector<byteRange> o2 = { byteRange(10,10), byteRange(30,10), byteRange(25,5)};
        result = result && (    c.overlapsAnyIn(o2)             );

        {
            byteRange fillThisRange(0,50);
            std::list<byteRange> blocks = { byteRange(10,10), byteRange(25,10), byteRange(40,0)};
            std::list<byteRange> copiesOfAddedBlocks;
            fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
            result = result && (  ! isNonDecreasingAndNonOverlapping(blocks)    );   //nondecreasing property broken by count 0 byteRange
            result = result && (    copiesOfAddedBlocks == std::list<byteRange>{ byteRange(0,10), byteRange(20,5), byteRange(35,15) });
        }
        {
            byteRange fillThisRange(0,50);
            std::list<byteRange> blocks;
            std::list<byteRange> copiesOfAddedBlocks;
            fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
            result = result && (    isNonDecreasingAndNonOverlapping(blocks)    );
            result = result && (    copiesOfAddedBlocks == std::list<byteRange>{ byteRange(0,50) } );
        }
        {
            byteRange fillThisRange(10,20);
            std::list<byteRange> blocks = { byteRange(0,15), byteRange(25,20)};
            std::list<byteRange> copiesOfAddedBlocks;
            fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
            result = result && (    isNonDecreasingAndNonOverlapping(blocks)    );
            result = result && (    copiesOfAddedBlocks == std::list<byteRange>{ byteRange(15,10) } );
        }
        {
            byteRange fillThisRange(0,20);
            std::list<byteRange> blocks = { byteRange(0,10), byteRange(10,0), byteRange(10,10)};
            std::list<byteRange> copiesOfAddedBlocks;
            fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
            result = result && (    isNonDecreasingAndNonOverlapping(blocks)    );
            result = result && (    copiesOfAddedBlocks.empty()                 );
        }


        return result;
    }
};

#endif // BYTERANGE_H
