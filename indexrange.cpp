#include "indexrange.h"

indexRange::indexRange()
    :   start(0),
        end(0)
{
}

indexRange::indexRange(unsigned int start, unsigned int end)
    :   start(start),
        end(end)
{
}

void indexRange::move(unsigned int newStart)
{
    unsigned int c = count();
    start = newStart;

    ASSERT(noSumOverflow(start,c));
    end = start+c;
}

unsigned int indexRange::count() const
{
    if (end <= start) {
        return 0;
    }

    return end - start;
}

bool indexRange::contains(const unsigned int &index) const
{
    return (    ( start <= index )
             && ( index <  end   )  );
}

bool indexRange::overlaps(const indexRange &r) const
{
    return 0 < getIntersection(r).count();
}

bool indexRange::operator==(const indexRange &r) const
{
    if ( (0 == count()) && (0 == r.count())) {
        return true;
    }

    return (start == r.start) && (end == r.end);
}

bool indexRange::operator!=(const indexRange &r) const
{
    return !( *this == r );
}

bool indexRange::operator< (const indexRange &r) const
{
    return start < r.start;
}

indexRange indexRange::getIntersection(const indexRange &r) const
{
    unsigned int start  = std::max(this->start, r.start);
    unsigned int end    = std::min(this->end,   r.end);

    if (end <= start) {
        return indexRange(0,0);  //no intersection, return size zero indexRange
    }

    return indexRange(start, end);
}

void indexRange::fillEmptySpaces(const indexRange &fillThisRange, std::list<indexRange> &blocks, std::list<indexRange> &copiesOfAddedBlocks)
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

bool indexRange::isExactAscendingPartition(const indexRange &partitionRange, const std::list<indexRange> &blocks)
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

