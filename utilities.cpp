#include "utilities.h"


/*static*/ unsigned int utilities::findStrongestRepetitionPeriod (
                const std::vector<unsigned char> &data,
                const byteRange inThisRange )
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


    byteRange searchRange;
    {
        ASSERT_NOT_NEGATIVE(data.size());
        byteRange dataRange(0, static_cast<unsigned int>(data.size()) );
        searchRange = inThisRange.getIntersection(dataRange);
    }

    if ( 0 == searchRange.count) {
        return 0;   //inThisRange doesn't overlap the data
    }

           //
          // for each possible byte value, record the previous (most recent) index at which it occurred
         //  UINT_MAX indicates that the byte value hasn't been seen yet
        //
    ASSERT_LE_UINT_MAX(searchRange.end());  //ensure that UINT_MAX won't be a valid index
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
    for (unsigned int i = searchRange.start; i < searchRange.end(); ++i) {

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
