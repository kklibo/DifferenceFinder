#include "offsetmetrics.h"

/*static*/ std::atomic_bool offsetMetrics::m_abort{false};

/*static*/ std::unique_ptr<rangeMatch>
            offsetMetrics::getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                    const std::vector<unsigned char>& target,
                                                    const unsigned int sourceRangeStart,
                                                    const indexRange sourceSearchRange,
                                                    const indexRange targetSearchRange
                                                    )
{

    ASSERT(0 < target.size());                          //vectors should have contents
    ASSERT(0 < source.size());
    ASSERT(sourceSearchRange.end <= source.size());     //ranges shouldn't exceed vectors
    ASSERT(targetSearchRange.end <= target.size());

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
        const unsigned int searchLimit = std::min(  sourceSearchRange.end - sourceRangeStart,
                                                    targetSearchRange.end - targetRangeStart  );

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
                                                const indexRange& targetSearchRange)
                                                -> rangeMatch {

        for (unsigned int i = targetSearchRange.start; i < targetSearchRange.end; ++i) {

            unsigned int alignmentRangeSize = getAlignmentRangeSizeAtIndices(sourceRangeStart, i);
            if (alignmentRangeSize > 1) {
                return rangeMatch(sourceRangeStart, i, alignmentRangeSize);
            }
        }

        //if this is reached, no alignment ranges were found
        return rangeMatch(0,0,0);
    };


    //look for an alignment range starting at sourceRangeStart
    rangeMatch alignmentRange = findAlignmentRange_fixedSourceIndex(sourceRangeStart, targetSearchRange);

    if (alignmentRange.byteCount) {
        return std::unique_ptr<rangeMatch>(new rangeMatch(alignmentRange));
    }

    //no match found
    return nullptr;
}

/*static*/ std::unique_ptr<rangeMatch>
            offsetMetrics::getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                    const std::vector<unsigned char>& target,
                                                    const indexRange sourceSearchRange,
                                                    const indexRange targetSearchRange
                                                    )
{
    //loop through source, looking for alignment ranges starting at each index until one is found
    for (unsigned int i = sourceSearchRange.start; i < sourceSearchRange.end; ++i) {
       // rangeMatch alignmentRange = findAlignmentRange_fixedSourceIndex(i, targetSearchRange);
        std::unique_ptr<rangeMatch> alignmentRange
            = offsetMetrics::getNextAlignmentRange( source, target,
                                                    i, sourceSearchRange, targetSearchRange);

        if (    nullptr != alignmentRange
             && alignmentRange->byteCount) {
            //return std::unique_ptr<rangeMatch>(new rangeMatch(alignmentRange));
            return alignmentRange;
        }
    }

    //no match found
    return nullptr;
}

/*static*/ std::unique_ptr<rangeMatch>
            offsetMetrics::getNextAlignmentRange(   const std::vector<unsigned char>& source,
                                                    const std::vector<unsigned char>& target,
                                                    const indexRange sourceSearchRange,
                                                    //this should be sorted by increasing start index
                                                    const std::list<indexRange>& targetSearchRanges
                                                    )
{

    ASSERT(indexRange::isNonDecreasingAndNonOverlapping(targetSearchRanges));

    //note: this checks all indices in sourceSearchRange against the current targetSearchRange
    //  before proceeding to the next targetSearchRange:
    //  this is not the same order as a full target search that skips (target) ranges


    for (const indexRange& targetSearchRange : targetSearchRanges) {

        std::unique_ptr<rangeMatch> result = getNextAlignmentRange(source, target, sourceSearchRange, targetSearchRange);

        if (result) {
            return result;
        }
    }

    return nullptr;
}

/*static*/ bool offsetMetrics::isNonMatchRangeExcludable(   const std::vector<unsigned char>& source,
                                                            const std::vector<unsigned char>& target,
                                                            const indexRange sourceNonMatchRange,
                                                            const indexRange targetNonMatchRange
                                                            )
{
    ASSERT(     sourceNonMatchRange.count()
             == targetNonMatchRange.count() );

    ASSERT_LE_UINT_MAX(source.size());
    ASSERT_LE_UINT_MAX(target.size());
    const indexRange sourceRange(0,static_cast<unsigned int>(source.size()));
    const indexRange targetRange(0,static_cast<unsigned int>(target.size()));

    //supplied non-matching ranges should actually be in the source and target
    ASSERT(0 < sourceRange.getIntersection(sourceNonMatchRange).count());
    ASSERT(0 < targetRange.getIntersection(targetNonMatchRange).count());


    const unsigned int lowerTestRangeCount =

    [&]() -> unsigned int {
        //extend the lower test range size up to 2x the size of nonMatchRange
        // constraints: start index shouldn't go negative
        //              start index shouldn't go outside dataRange
        auto get2XWithConstraints =
        [](const indexRange& nonMatchRange, const indexRange& dataRange)-> unsigned int
        {
            const unsigned int lowerTestRangeStart
                    = utilities::subtractClampToZero( nonMatchRange.start, 2*nonMatchRange.count() );

            indexRange lowerRange( lowerTestRangeStart, nonMatchRange.start );

            lowerRange = lowerRange.getIntersection(dataRange);

            return lowerRange.count();
        };

        const unsigned int sourceLowerTestRangeCount
            = get2XWithConstraints(sourceNonMatchRange, sourceRange);

        const unsigned int targetLowerTestRangeCount
            = get2XWithConstraints(targetNonMatchRange, targetRange);

        //choose the lesser of the two range sizes:
        // only equal-sized ranges can be compared index-to-index
        return std::min(sourceLowerTestRangeCount,
                        targetLowerTestRangeCount);
    } ();

    const unsigned int upperTestRangeCount =

    [&]() -> unsigned int {
        //extend the upper test range size up to 2x the size of nonMatchRange
        // constraints: end index shouldn't overflow unsigned int
        //              end index shouldn't go beyond the end of dataRange
        auto get2XWithConstraints =
        [](const indexRange& nonMatchRange, const indexRange& dataRange)-> unsigned int
        {
            const unsigned int upperTestRangeEnd
                    = utilities::addClampToMax( nonMatchRange.end, 2*nonMatchRange.count() );

            indexRange upperRange( nonMatchRange.end, upperTestRangeEnd );

            upperRange = upperRange.getIntersection(dataRange);

            return upperRange.count();
        };

        const unsigned int sourceUpperTestRangeCount
            = get2XWithConstraints(sourceNonMatchRange, sourceRange);

        const unsigned int targetUpperTestRangeCount
            = get2XWithConstraints(targetNonMatchRange, targetRange);

        //choose the lesser of the two range sizes:
        // only equal-sized ranges can be compared index-to-index
        return std::min(sourceUpperTestRangeCount,
                        targetUpperTestRangeCount);
    } ();


    ASSERT(sourceNonMatchRange.start >= lowerTestRangeCount);
    const indexRange sourceLowerTestRange(sourceNonMatchRange.start - lowerTestRangeCount,
                                          sourceNonMatchRange.start);

    ASSERT(targetNonMatchRange.start >= lowerTestRangeCount);
    const indexRange targetLowerTestRange(targetNonMatchRange.start - lowerTestRangeCount,
                                          targetNonMatchRange.start);


    ASSERT(noSumOverflow(sourceNonMatchRange.end, upperTestRangeCount));
    const indexRange sourceUpperTestRange(sourceNonMatchRange.end,
                                          sourceNonMatchRange.end + upperTestRangeCount);

    ASSERT(noSumOverflow(targetNonMatchRange.end, upperTestRangeCount));
    const indexRange targetUpperTestRange(targetNonMatchRange.end,
                                          targetNonMatchRange.end + upperTestRangeCount);

    ASSERT(    sourceLowerTestRange.count()
            == targetLowerTestRange.count());
    const unsigned int
    lowerMatchCount = utilities::countMatchingIndices(  source,
                                                        target,
                                                        sourceLowerTestRange,
                                                        targetLowerTestRange);
    ASSERT(    sourceUpperTestRange.count()
            == targetUpperTestRange.count());
    const unsigned int
    upperMatchCount = utilities::countMatchingIndices(  source,
                                                        target,
                                                        sourceUpperTestRange,
                                                        targetUpperTestRange);

    if (lowerMatchCount < sourceNonMatchRange.count()) {
    //if (sourceLowerTestRange.count() - lowerMatchCount >= sourceNonMatchRange.count()) {
        return true;
    }

    if (upperMatchCount < sourceNonMatchRange.count()) {
    //if (sourceUpperTestRange.count() - upperMatchCount >= sourceNonMatchRange.count()) {
        return true;
    }

    return false;
}

/*static*/ bool offsetMetrics::truncateAlignmentRange(  const std::vector<unsigned char>& data1,
                                                        const std::vector<unsigned char>& data2,
                                                        rangeMatch& alignmentRange
                                                        )
{
    LOG.Debug("truncate alignment range");

    std::list<indexRange> file1_matches;
    std::list<indexRange> file1_differences;
    std::list<indexRange> file2_matches;
    std::list<indexRange> file2_differences;

    offsetMetrics::getAlignmentRangeDiff( data1, data2, alignmentRange,
                                          file1_matches,
                                          file1_differences,
                                          file2_matches,
                                          file2_differences);

    ASSERT(    file1_matches.size()
            == file2_matches.size());

    ASSERT(    file1_differences.size()
            == file2_differences.size());

    std::list<indexRange>::const_iterator i1 = file1_differences.begin();
    std::list<indexRange>::const_iterator i2 = file2_differences.begin();

    while(i1 != file1_differences.end()) {

        ASSERT(    i1->count()
                == i2->count());

        if (offsetMetrics::isNonMatchRangeExcludable(data1, data2, *i1, *i2)) {

            ASSERT(    i1->start - alignmentRange.startIndexInFile1
                    == i2->start - alignmentRange.startIndexInFile2);

            alignmentRange.byteCount = i1->start - alignmentRange.startIndexInFile1;
            return true;
        }

        ++i1;
        ++i2;
    }
    return false;
}

/*static*/ void offsetMetrics::getAlignmentRangeDiff(   const std::vector<unsigned char>& file1,
                                                        const std::vector<unsigned char>& file2,
                                                        const rangeMatch& alignmentRange,
                                                        std::list<indexRange>& file1_matches,
                                                        std::list<indexRange>& file1_differences,
                                                        std::list<indexRange>& file2_matches,
                                                        std::list<indexRange>& file2_differences
                                                        )
{
    if ( !alignmentRange.byteCount ) {
        return;
    }

    ASSERT(file1.size() >= alignmentRange.getEndInFile1());
    ASSERT(file2.size() >= alignmentRange.getEndInFile2());


    auto addToRange = [](const unsigned int index, indexRange& activeRange) {
        //open a new active range (if not already active)
        if (!activeRange.count()) {
            activeRange.move(index);
        }

        //and increment the range size
        ASSERT(noSumOverflow(activeRange.end,1));
        ++(activeRange.end);
    };

    auto closeRangeIfActive = [](indexRange& range, std::list<indexRange>& addClosedRangeHere) {
        //output and close range (if active)
        if (range.count()) {
            addClosedRangeHere.emplace_back(range);
            range = indexRange(0,0);
        }
    };


    //these ranges are progressively expanded as the alignment range is traversed,
    // then added to output lists when their end is found
    indexRange file1MatchRange(0,0);
    indexRange file1DifferenceRange(0,0);
    indexRange file2MatchRange(0,0);
    indexRange file2DifferenceRange(0,0);

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

        std::list<indexRange> targetSearchRanges;
        {
            //find valid target search ranges (temp code, avoid copying list?)
            std::list<indexRange> alignmentRangesInTarget;

            for (rangeMatch& alignmentRange : alignmentRanges) {
                ASSERT(noSumOverflow(                alignmentRange.startIndexInFile2,alignmentRange.byteCount));
                alignmentRangesInTarget.emplace_back(alignmentRange.startIndexInFile2,
                                                     alignmentRange.startIndexInFile2+alignmentRange.byteCount);
            }

            //fillEmptySpaces requires a nondecreasing and nonoverlapping block list
            alignmentRangesInTarget.sort();

            ASSERT_LE_UINT_MAX(data2.size());
            indexRange::fillEmptySpaces(indexRange(0, static_cast<unsigned int>(data2.size())), alignmentRangesInTarget, targetSearchRanges);
        }

        ASSERT_LE_UINT_MAX(data1.size());
        indexRange sourceSearchRange(sourceStartIndex, static_cast<unsigned int>(data1.size()));

        std::unique_ptr<rangeMatch> rangeResult
            = offsetMetrics::getNextAlignmentRange(data1, data2, sourceSearchRange, targetSearchRanges);

        if (rangeResult){

            truncateAlignmentRange(data1, data2, *rangeResult);

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
