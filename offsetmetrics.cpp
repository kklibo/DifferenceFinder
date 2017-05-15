#include "offsetmetrics.h"

/*static*/ std::atomic_bool offsetMetrics::m_abort{false};

/*static*/ std::unique_ptr<rangeMatch>
            offsetMetrics::getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                    const std::vector<unsigned char>& target,
                                                    const byteRange sourceSearchRange,
                                                    const byteRange targetSearchRange
                                                    )
{

    ASSERT(0 < target.size());                          //vectors should have contents
    ASSERT(0 < source.size());
    ASSERT(sourceSearchRange.end() <= source.size());   //ranges shouldn't exceed vectors
    ASSERT(targetSearchRange.end() <= target.size());

    //returns the size of the alignment range starting at these indices in source and target:
    //an alignment range contains >50% index-to-index matching bytes between source and target,
    // and has matching bytes at its lowest and highest indices (i.e., non-matches on edges are excluded)
    auto getAlignmentRangeSizeAtIndices = [&source, &target, &sourceSearchRange, &targetSearchRange]
                                          (const unsigned int sourceRangeStart,
                                           const unsigned int targetRangeStart)
                                           -> unsigned int {

        unsigned int rangeSize = 0;     //the result
        unsigned int matchCount = 0;    //the number of paired indices between source and target that contain the same byte


        ASSERT(sourceSearchRange.contains(sourceRangeStart));   //make sure start indices are within search ranges
        ASSERT(targetSearchRange.contains(targetRangeStart));

        //limit search to within source & target ranges
        const unsigned int searchLimit = std::min(  sourceSearchRange.end() - sourceRangeStart,
                                                    targetSearchRange.end() - targetRangeStart  );

        for (unsigned int i = 0; i < searchLimit; ++i) {

            unsigned int count = i + 1;   //indices compared so far

            //test for matching bytes between these ranges
            if (    source[sourceRangeStart + i]
                 == target[targetRangeStart + i] ) {

                rangeSize = count;
                ++matchCount;
            }
            else {

                //if the match ratio just dropped below 50%, the range has ended (or never started)
                if (matchCount*2 < count) {
                    break;
                }
            }
        }

        //either the loop was broken because the match ratio dropped below 50%,
        // or the end of source or target has been reached
        return rangeSize;
    };


    auto findAlignmentRange_fixedSourceIndex = [&source, &target, &getAlignmentRangeSizeAtIndices]
                                               (const unsigned int sourceRangeStart,
                                                const byteRange& targetSearchRange)
                                                -> rangeMatch {

        for (unsigned int i = targetSearchRange.start; i < targetSearchRange.end(); ++i) {

            unsigned int alignmentRangeSize = getAlignmentRangeSizeAtIndices(sourceRangeStart, i);
            if (alignmentRangeSize > 1) {
                return rangeMatch(sourceRangeStart, i, alignmentRangeSize);
            }
        }

        //if this is reached, no alignment ranges were found
        return rangeMatch(0,0,0);
    };


    //loop through source, looking for alignment ranges starting at each index until one is found
    for (unsigned int i = sourceSearchRange.start; i < sourceSearchRange.end(); ++i) {
        rangeMatch alignmentRange = findAlignmentRange_fixedSourceIndex(i, targetSearchRange);

        if (alignmentRange.byteCount) {
            return std::unique_ptr<rangeMatch>(new rangeMatch(alignmentRange));
        }
    }

    //no match found
    return nullptr;
}

/*static*/ std::unique_ptr<rangeMatch>
            offsetMetrics::getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                    const std::vector<unsigned char>& target,
                                                    const byteRange sourceSearchRange,
                                                    //this should be sorted by increasing start index
                                                    const std::list<byteRange>& targetSearchRanges
                                                    )
{

    ASSERT(byteRange::isNonDecreasingAndNonOverlapping(targetSearchRanges));

    //note: this checks all indices in sourceSearchRange against the current targetSearchRange
    //  before proceeding to the next targetSearchRange:
    //  this is not the same order as a full target search that skips (target) ranges


    for (const byteRange& targetSearchRange : targetSearchRanges) {

        std::unique_ptr<rangeMatch> result = getNextAlignmentRange(source, target, sourceSearchRange, targetSearchRange);

        if (result) {
            return result;
        }
    }

    return nullptr;
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


    auto addToRange = [](const unsigned int index, byteRange& activeRange) {
        //open a new active range (if not already active)
        if (!activeRange.count) {
            activeRange.start = index;
        }

        //and increment the range size
        ++activeRange.count;
    };

    auto closeRangeIfActive = [](byteRange& range, std::list<byteRange>& addClosedRangeHere) {
        //output and close range (if active)
        if (range.count) {
            addClosedRangeHere.emplace_back(range);
            range = byteRange(0,0);
        }
    };


    //these ranges are progressively expanded as the alignment range is traversed,
    // then added to output lists when their end is found
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
            addToRange          (file1Index, file1MatchRange);
            closeRangeIfActive  (file1DifferenceRange, file1_differences);
            addToRange          (file2Index, file2MatchRange);
            closeRangeIfActive  (file2DifferenceRange, file2_differences);

        } else {

            //bytes are different:
            addToRange          (file1Index, file1DifferenceRange);
            closeRangeIfActive  (file1MatchRange, file1_matches);
            addToRange          (file2Index, file2DifferenceRange);
            closeRangeIfActive  (file2MatchRange, file2_matches);
        }
    }

    //close any open ranges
    closeRangeIfActive(file1MatchRange,         file1_matches);
    closeRangeIfActive(file1DifferenceRange,    file1_differences);
    closeRangeIfActive(file2MatchRange,         file2_matches);
    closeRangeIfActive(file2DifferenceRange,    file2_differences);
}

/*static*/
std::unique_ptr<offsetMetrics::results>
offsetMetrics::doCompare(   const std::vector<unsigned char>& data1,
                            const std::vector<unsigned char>& data2 )
{
    m_abort = false; //clear abort flag

    auto Results = std::unique_ptr<offsetMetrics::results>( new offsetMetrics::results );

    if (!data1.size() || !data2.size()) {
        Results->internalError = true;
        return Results;
    }




    unsigned int sourceStartIndex = 0;
    std::list<rangeMatch> alignmentRanges;

    while(1) {

        if (m_abort) {
            Results->aborted = true;
            return Results;
        }

        std::list<byteRange> targetSearchRanges;
        {
            //find valid target search ranges (temp code, avoid copying list?)
            std::list<byteRange> alignmentRangesInTarget;

            for (rangeMatch& alignmentRange : alignmentRanges) {
                alignmentRangesInTarget.emplace_back(alignmentRange.startIndexInFile2, alignmentRange.byteCount);
            }

            //fillEmptySpaces requires a nondecreasing and nonoverlapping block list
            alignmentRangesInTarget.sort();

            ASSERT_LE_UINT_MAX(data2.size());
            byteRange::fillEmptySpaces(byteRange(0, static_cast<unsigned int>(data2.size())), alignmentRangesInTarget, targetSearchRanges);
        }

        ASSERT_LE_UINT_MAX(data1.size());
        byteRange sourceSearchRange(sourceStartIndex, static_cast<unsigned int>(data1.size()) - sourceStartIndex);

        std::unique_ptr<rangeMatch> rangeResult
            = offsetMetrics::getNextAlignmentRange(data1, data2, sourceSearchRange, targetSearchRanges);

        if (rangeResult){
            rangeMatch range = *rangeResult.release();
            LOG.Info(QString("getNextAlignmentRange: %1, %2; %3")
                            .arg(range.startIndexInFile1)
                            .arg(range.startIndexInFile2)
                            .arg(range.byteCount));

            sourceStartIndex = range.getEndInFile1();

            alignmentRanges.push_back(range);
        }
        else
        {
            LOG.Info(QString("getNextAlignmentRange returned nullptr"));
            break;
        }
    }

    for (const rangeMatch& alignmentRange : alignmentRanges) {

        if (m_abort) {
            Results->aborted = true;
            return Results;
        }

        offsetMetrics::getAlignmentRangeDiff(data1, data2, alignmentRange,
                                                        Results->file1_matches,
                                                        Results->file1_differences,
                                                        Results->file2_matches,
                                                        Results->file2_differences   );
    }

    return Results;
}

/*static*/ void offsetMetrics::abort()
{
    m_abort = true;
}
