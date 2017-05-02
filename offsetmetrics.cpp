#include "offsetmetrics.h"

offsetMetrics::offsetMetrics()
{

}

/*static*/ std::unique_ptr<std::vector<int>> offsetMetrics::getOffsetMap(const std::vector<unsigned char>& source,
                                                                         const std::vector<unsigned char>& target,
                                                                         const unsigned int sourceStartIndex,
                                                                         const unsigned int targetStartIndex
                                                                         )
{
    //TODO: ensure integer range safety (including interactions with reserved INT_MAX value)
    //TODO? (after tests are set up): return result object that offers source aligned indices w/ less memory allocated (encapsulate offset, allocate less memory)

    //create offset map (to be returned); same size as source
    std::unique_ptr<std::vector<int>> offsetMap(new std::vector<int>(source.size()));

    //the next index in target at which to continue searching for each possible byte value
    std::vector<unsigned int> targetSearchIndices(256, targetStartIndex);    //size 256, initialized to targetStartIndex


    for (unsigned int i = sourceStartIndex; i < source.size(); ++i) {

        //search for current source byte in target
        const unsigned char sourceByte = source[i];

        //look up where to start the search
        unsigned int nextTargetIndex = targetSearchIndices[sourceByte];

        for ( ; nextTargetIndex < target.size(); ++nextTargetIndex) {

            if (sourceByte == target[nextTargetIndex]) {
                //match found
                //record offset
                (*offsetMap.get())[i] = nextTargetIndex - i;

                //the next search for this byte will continue after this index
                targetSearchIndices[sourceByte] = nextTargetIndex + 1;

                break;
            }

        }

        //if nextTargetIndex has reached the end of target, there are no (more) matches for this sourceByte
        if (nextTargetIndex >= target.size()) {
            (*offsetMap.get())[i] = INT_MAX;
        }
    }

    return offsetMap;

}


//inefficient version
/*static*/ std::unique_ptr<rangeMatch>
            offsetMetrics::getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                    const std::vector<unsigned char>& target,
                                                    const unsigned int sourceStartIndex,
                                                    const unsigned int targetStartIndex
                                                    )
{
    auto getOffset = [&source, &target] (   const unsigned int sourceIndex,
                                                  unsigned int nextTargetIndex ) -> int
    {
        //search for current source byte in target
        const unsigned char sourceByte = source[sourceIndex];


        for ( ; nextTargetIndex < target.size(); ++nextTargetIndex) {

            if (sourceByte == target[nextTargetIndex]) {
                //match found
                return nextTargetIndex - sourceIndex;
            }
        }

        //if nextTargetIndex has reached the end of target, there are no matches for this sourceByte
        return INT_MAX;
    };



    unsigned int rangeStart;
    int offset;

    if (target.size() < 3){
        return nullptr;
    }


//switch to checking w/ just offset test
//use offset finder for first hit only


    //find first block of 3 offset values w/ at least 2 equal offsets
    {
        unsigned int i;
        for (i = sourceStartIndex; i < source.size(); ++i) {

            offset = getOffset(i, targetStartIndex);

            if (offset != INT_MAX) {
                //offset found, check 2 of 3 match
                if (    (source[i + 1] == target[i + offset + 1])
                     || (source[i + 2] == target[i + offset + 2]))
                {
                    break;  //block found
                }
            }
        }

        if (i >= source.size()) {
            return nullptr; //reached the end without finding an alignment range
        }

        rangeStart = i;

    }

    unsigned int majorityBytes = 0;
    unsigned int highestMatchIndex;

    //count total and expand until end of file or <=50%
    unsigned int rangeEnd = source.size();
    if(rangeEnd > target.size() - offset){
        rangeEnd = target.size() - offset;
    }//rangeEnd = min of 2 limits

    for (unsigned int i = rangeStart; i < source.size() && i + offset < target.size(); ++i) {
        if (source[i] == target[i + offset]) {
            highestMatchIndex = i;
            ++majorityBytes;
        } else {
            unsigned int count = 1 + i - rangeStart;
            if (majorityBytes*2 <= count) {
                rangeEnd = i;
                break;
            }
        }
    }

    if (rangeStart == rangeEnd) {
        return nullptr;
    }

    //trim range
    //TODO?: add mode that prefers no trim: instead try full file sync with just replacements

    //byteRange rangeInFile1(rangeStart, 1+highestMatchIndex-rangeStart);

    //return std::unique_ptr<byteRange>(new byteRange(rangeStart, rangeEnd-rangeStart));
    //return std::unique_ptr<byteRange>(new byteRange(rangeStart, 1+highestMatchIndex-rangeStart));
    //return std::unique_ptr<alignmentRange>(new alignmentRange(rangeInFile1, offset));
    return std::unique_ptr<rangeMatch>(new rangeMatch(rangeStart, rangeStart + offset, 1+highestMatchIndex-rangeStart));

}

/*static*/ void offsetMetrics::getAlignmentRangeDiff(   const std::vector<unsigned char>& file1,
                                                        const std::vector<unsigned char>& file2,
                                                        const rangeMatch& alignmentRange,
                                                        std::list<byteRange>& file1_matches,
                                                        std::list<byteRange>& file1_differences,
                                                        std::list<byteRange>& file2_matches,
                                                        std::list<byteRange>& file2_differences
                                                        )
{
    if ( !alignmentRange.byteCount ) {
        return;
    }

    ASSERT(file1.size() >= alignmentRange.getEndInFile1());
    ASSERT(file2.size() >= alignmentRange.getEndInFile2());


    auto closeRange = [](byteRange& range, std::list<byteRange>& addClosedRangeHere) {
        //output and close range (if open)
        if (range.count) {
            addClosedRangeHere.emplace_back(range);
            range = byteRange(0,0);
        }
    };

    //TODO: capture closeRange by reference here?
    auto doNextIndexDiff = [&closeRange](const unsigned int index, byteRange& activeRange, byteRange& inactiveRange, std::list<byteRange>& addInactiveRangeHere) {

        //output and close inactive range (if open)
        closeRange(inactiveRange, addInactiveRangeHere);
        /*if (inactiveRange.count) {
            addInactiveRangeHere.emplace_back(inactiveRange);
            inactiveRange = byteRange(0,0);
        }*/

        //and open a new active range (if not already open)
        if (!activeRange.count) {
            activeRange.start = index;
        }

        //and increment the range size
        ++activeRange.count;
    };


    //these ranges are progressively expanded as the alignment range is traversed,
    // then added to output lists when complete
    byteRange file1MatchRange(0,0);
    byteRange file1DifferenceRange(0,0);
    byteRange file2MatchRange(0,0);
    byteRange file2DifferenceRange(0,0);

    for (unsigned int i = 0; i < alignmentRange.byteCount; ++i) {

        unsigned int file1Index = alignmentRange.startIndexInFile1 + i;
        unsigned int file2Index = alignmentRange.startIndexInFile2 + i;

        if (    file1[file1Index]
             == file2[file2Index] ) {

            //bytes are equal:
            doNextIndexDiff(file1Index, file1MatchRange, file1DifferenceRange, file1_differences);
            doNextIndexDiff(file2Index, file2MatchRange, file2DifferenceRange, file2_differences);

        } else {

            //bytes are different:
            doNextIndexDiff(file1Index, file1DifferenceRange, file1MatchRange, file1_matches);
            doNextIndexDiff(file2Index, file2DifferenceRange, file2MatchRange, file2_matches);

        }
    }

    //close any open ranges
    closeRange(file1MatchRange,         file1_matches);
    closeRange(file1DifferenceRange,    file1_differences);
    closeRange(file2MatchRange,         file2_matches);
    closeRange(file2DifferenceRange,    file2_differences);


}

//inefficient version (this didn't work, the offset map needs constant refresh)
/*static*//* std::unique_ptr<byteRange> offsetMetrics::getNextAlignmentRange( const std::vector<unsigned char>& source,
                                                                            const std::vector<unsigned char>& target,
                                                                            const unsigned int sourceStartIndex,
                                                                            const unsigned int targetStartIndex
                                                                            )
{
    auto offsetMapResult = getOffsetMap(source, target, sourceStartIndex, targetStartIndex);
    if (nullptr == offsetMapResult) {
        return nullptr;
    }

    unsigned int rangeStart;

    int offsetValue;
    std::vector<int> offsetMap = *offsetMapResult.release();

    //find first block of 3 offset values w/ at least 2 equal offsets
    {
        unsigned int i;
        for (i = sourceStartIndex + (3 - 1); i < source.size(); ++i) {

            if (offsetMap[i-2] == offsetMap[i-1]) {
                offsetValue = offsetMap[i-2];
                break;
            }

            if (offsetMap[i-2] == offsetMap[i-0]) {
                offsetValue = offsetMap[i-2];
                break;
            }

            if (offsetMap[i-1] == offsetMap[i-0]) {
                offsetValue = offsetMap[i-1];
                break;
            }
        }

        if (i >= source.size()) {
            return nullptr; //reached the end without finding an alignment range
        }
        rangeStart = i-2;
    }

    unsigned int majorityBytes = 0;

    //count total and expand until end of file or <=50%
    for (unsigned int i = rangeStart; i < rangeStart+3; ++i) {
        if (offsetMap[i] == offsetValue) {
            ++majorityBytes;
        }
    }

    unsigned int rangeEnd = source.size();
    for (unsigned int i = rangeStart+3; i < source.size(); ++i) {
        if (offsetMap[i] == offsetValue) {
            ++majorityBytes;
        } else {
            unsigned int count = 1 + i - rangeStart;
            if (majorityBytes*2 <= count) {
                rangeEnd = i;
                break;
            }
        }
    }

    if (rangeStart == rangeEnd) {
        return nullptr;
    }

    return std::unique_ptr<byteRange>(new byteRange(rangeStart, rangeEnd-rangeStart));

    //trim range

}
*/


/*static*//* std::unique_ptr<byteRange> offsetMetrics::getNextAlignmentRange( const std::vector<unsigned char>& source,
                                                                            const std::vector<unsigned char>& target,
                                                                            const unsigned int sourceStartIndex,
                                                                            const unsigned int targetStartIndex
                                                                            )
{
    //2 out of 3 consecutive bytes in the offset map must have the same offset value to detect an alignment range
    const unsigned int countNeeded  = 2;
    const unsigned int outOf        = 3;


    //encapsulated circular buffer of offset map values
    std::vector<unsigned char> buffer(outOf,0);
    unsigned int bufferZero = 0;

    auto addNextOffset = [&bufferZero, &buffer, outOf](unsigned char offset){

        ++bufferZero;
        if (bufferZero >= outOf) {
            bufferZero = 0;
        }

        buffer[bufferZero] = offset;
    };

    auto getOffsets = [](unsigned int index){

    };


    if (source.size() < outOf) {
        //source is too small
        return nullptr;
    }


    for (unsigned int i = sourceStartIndex + (outOf - 1); i < source.size(); ++i) {

        if()

    }

}*/
