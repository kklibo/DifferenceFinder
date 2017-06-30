#include "utilities.h"


/*static*/ unsigned int utilities::findStrongestRepetitionPeriod (
                const std::vector<unsigned char> &data,
                const indexRange inThisRange )
{
    /*
        tries to find the period of a trend of repetition in this data
        by finding the most frequent offset between occurrences of the same byte.

        step through the entire range once
            for each byte:
                if this byte value has occurred before
                    determine offset from last occurrence and add one to that offset count

        the most frequent offset is the strongest repetition period
    */


    indexRange searchRange;
    {
        ASSERT_LE_UINT_MAX(data.size());
        indexRange dataRange(0, static_cast<unsigned int>(data.size()) );
        searchRange = inThisRange.getIntersection(dataRange);
    }

    if ( 0 == searchRange.count()) {
        return 0;   //inThisRange doesn't overlap the data
    }

           //
          // for each possible byte value, record the previous (most recent) index at which it occurred
         //  UINT_MAX indicates that the byte value hasn't been seen yet
        //
    ASSERT_LE_UINT_MAX(searchRange.end);  //ensure that UINT_MAX won't be a valid index
      //
    std::vector<unsigned int> previousIndex(256, UINT_MAX); //size 256, initialized to UINT_MAX
    //

         //
        // when a byte value is read (other than the first time),
       //  the offset between its index and the previous index at which its value was read is recorded
      //
    std::map<unsigned int, unsigned int> offsetCount;   //<offset, count>
    //


    //traverse the search range, recording offsets between repetitions
    for (unsigned int i = searchRange.start; i < searchRange.end; ++i) {

        if (UINT_MAX != previousIndex[data[i]]) {
            //this value has a previous occurrence
            unsigned int offset = i - previousIndex[data[i]];

            ++offsetCount[offset];  //increment the count for this offset
        }

        previousIndex[data[i]] = i; //record this value's occurrence
    }


    //find the most common offset
    std::pair<unsigned int, unsigned int> highestValue(0,0);
    for (std::map<unsigned int, unsigned int>::const_iterator i = offsetCount.begin(); i != offsetCount.end(); ++i) {

        if (i->second > highestValue.second) {
            highestValue = *i;
        }
    }

    //return the most common offset
    return highestValue.first;

}


/*static*/  std::unique_ptr<std::vector<unsigned char>>
            utilities::createOffsetByteMap (
                const std::vector<unsigned char>& data,
                const indexRange inThisRange )
{

    indexRange searchRange;
    {
        ASSERT_LE_UINT_MAX(data.size());
        indexRange dataRange(0, static_cast<unsigned int>(data.size()) );
        searchRange = inThisRange.getIntersection(dataRange);
    }

    if ( 0 == searchRange.count()) {
        return 0;   //inThisRange doesn't overlap the data
    }

           //
          // for each possible byte value, record the previous (most recent) index at which it occurred
         //  UINT_MAX indicates that the byte value hasn't been seen yet
        //
    ASSERT_LE_UINT_MAX(searchRange.end);  //ensure that UINT_MAX won't be a valid index
      //
    std::vector<unsigned int> previousIndex(256, UINT_MAX); //size 256, initialized to UINT_MAX
    //


    std::unique_ptr<std::vector<unsigned char>> offsetByteMap(new std::vector<unsigned char>(data.size()));

    //traverse the search range, recording offsets between repetitions
    for (unsigned int i = searchRange.start; i < searchRange.end; ++i) {

        if (UINT_MAX != previousIndex[data[i]]) {
            //this value has a previous occurrence
            unsigned int offset = i - previousIndex[data[i]];

            offset = std::min(offset, 255u);
            (*offsetByteMap)[i] = static_cast<unsigned char>(offset);

        } else {
            //no previous occurrence
            (*offsetByteMap)[i] = 0;
        }

        previousIndex[data[i]] = i; //record this value's occurrence
    }

    return offsetByteMap;
}


/*static*/  std::unique_ptr<std::vector<unsigned char>>
            utilities::createCrossFileOffsetByteMap (
                const std::vector<unsigned char>& source,
                const indexRange sourceRange,
                const std::vector<unsigned char>& target,
                const indexRange targetRange,
                const bool runBackwards )
{

    indexRange sourceSearchRange;
    {
        ASSERT_LE_UINT_MAX(source.size());
        indexRange dataRange(0, static_cast<unsigned int>(source.size()) );
        sourceSearchRange = sourceRange.getIntersection(dataRange);
    }

    indexRange targetSearchRange;
    {
        ASSERT_LE_UINT_MAX(target.size());
        indexRange dataRange(0, static_cast<unsigned int>(target.size()) );
        targetSearchRange = targetRange.getIntersection(dataRange);
    }

           //
          // for each possible byte value, record the previous (most recent) index at which it occurred
         //  UINT_MAX indicates that the byte value hasn't been seen yet
        //
    ASSERT_LE_UINT_MAX(sourceSearchRange.end);  //ensure that UINT_MAX won't be a valid index
      //
    std::vector<unsigned int> previousIndex(256, UINT_MAX); //size 256, initialized to UINT_MAX
    //


    std::unique_ptr<std::vector<unsigned char>> offsetByteMap(new std::vector<unsigned char>(source.size(), 0xFEu));

    unsigned int searchCount;
    {
        indexRange searchRange = sourceSearchRange.getIntersection(targetSearchRange);
        searchCount = searchRange.count();

        if (0 == searchCount) {
            //nothing to search:
            //either sourceSearchRange or targetSearchRange are size 0,
            // or they don't overlap
            return nullptr;
        }
    }

    //to ensure that offsets are positive
    bool sourceStartsBeforeOrWithTarget;
    {
        if (!runBackwards) {
            sourceStartsBeforeOrWithTarget = sourceSearchRange.start < targetSearchRange.start;
        } else {
            sourceStartsBeforeOrWithTarget = sourceSearchRange.end > targetSearchRange.end;
        }
    }

    //traverse the search range, recording offsets between repetitions
    for (unsigned int i = 0; i < searchCount; ++i) {

        unsigned int sourceIndex;
        unsigned int targetIndex;
        unsigned int offset;

        if (!runBackwards) {
            //reading start to end

            sourceIndex = sourceSearchRange.start + i;
            targetIndex = targetSearchRange.start + i;

            previousIndex[target[targetIndex]] = targetIndex; //record this value's occurrence

            if (sourceStartsBeforeOrWithTarget) {
                offset = previousIndex[source[sourceIndex]] - sourceIndex;
            } else {
                offset = sourceIndex - previousIndex[source[sourceIndex]];
            }

        } else {
            //reading end to start

            sourceIndex = sourceSearchRange.end - 1 - i;
            targetIndex = targetSearchRange.end - 1 - i;

            previousIndex[target[targetIndex]] = targetIndex; //record this value's occurrence

            if (sourceStartsBeforeOrWithTarget) {
                offset = sourceIndex - previousIndex[source[sourceIndex]];
            } else {
                offset = previousIndex[source[sourceIndex]] - sourceIndex;
            }
        }

        if (UINT_MAX != previousIndex[source[sourceIndex]]) {
            //this value has been found in the target

            offset = std::min(offset, 0xFDu);//255u);
            (*offsetByteMap)[sourceIndex] = static_cast<unsigned char>(offset);

        } else {
            //no previous occurrence
            (*offsetByteMap)[sourceIndex] = 0xFFu;
        }

    }

    return offsetByteMap;
}

/*static*/
unsigned int
utilities::subtractClampToZero (
        const unsigned int& value,
        const unsigned int& subtractThis)
{
    if (value <= subtractThis) {
        return 0;
    } else {
        return value - subtractThis;
    }
}


/*static*/
unsigned int
utilities::addClampToMax (
        const unsigned int& value,
        const unsigned int& addThis)
{
    if (UINT_MAX - addThis <= value) {
        return UINT_MAX;
    } else {
        return value + addThis;
    }
}
