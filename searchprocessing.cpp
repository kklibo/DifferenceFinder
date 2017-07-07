#include "searchprocessing.h"
/*
searchProcessing::searchProcessing()
{

}
*/

/*static*/
std::unique_ptr<searchProcessing::searchState>
    searchProcessing::doSearch( const std::vector<unsigned char>& dataSet1,
                                const std::vector<unsigned char>& dataSet2,
                                const std::shared_ptr<searchState> parentState,
                                const searchAction action,
                                const indexRange dataSet1Range,
                                const indexRange dataSet2Range )
{


    std::unique_ptr<searchState> result(new searchState);
    result->actionTaken = action;
    result->parentState = parentState;

    if      (action == searchAction::none) {
        FAIL();
        return nullptr;
    }
    else if (action == searchAction::advanceBothPointersBy1) {

        if (    (1 >= dataSet1Range.count())
             || (1 >= dataSet2Range.count()) ) {

            //this advancement would hit the end of one of the search ranges
            return nullptr;
        }

        result->dataSet1NextRange = dataSet1Range;
        result->dataSet2NextRange = dataSet2Range;

        ASSERT(dataSet1Range.start < UINT_MAX - 1);
        ASSERT(dataSet2Range.start < UINT_MAX - 1);

        result->dataSet1NextRange.start = dataSet1Range.start + 1;
        result->dataSet2NextRange.start = dataSet2Range.start + 1;


        result->newAlignmentRange = nullptr;


        return result;

    }
    else if (    (action == searchAction::advanceDataSet1PointerUntilMatchRange)
              || (action == searchAction::advanceDataSet2PointerUntilMatchRange) )
    {

        if (    (0 == dataSet1Range.count())
             || (0 == dataSet2Range.count()) ) {

            return nullptr;
        }

        unsigned int dS1Index = dataSet1Range.start;
        unsigned int dS2Index = dataSet2Range.start;

        if (action == searchAction::advanceDataSet1PointerUntilMatchRange) {

            for ( ; dS1Index < dataSet1Range.end; ++dS1Index) {

                if (dataSet1[dS1Index] == dataSet2[dS2Index]) {
                    //match found
                    break;
                }
            }

        } else {   // searchAction::advanceDataSet2PointerUntilMatchRange

            for ( ; dS2Index < dataSet2Range.end; ++dS2Index) {

                if (dataSet1[dS1Index] == dataSet2[dS2Index]) {
                    //match found
                    break;
                }
            }
        }


        if (dS1Index == dataSet1Range.end) {
            //no match found
            return nullptr;
        }

        //set range starts
        result->newAlignmentRange.reset(new alignmentRange);
        result->newAlignmentRange->inDataSet1.start = dS1Index;
        result->newAlignmentRange->inDataSet2.start = dS2Index;

        //find length of range

        while (    (dS1Index < dataSet1Range.end)
                && (dS2Index < dataSet2Range.end) ) {

            if (dataSet1[dS1Index] == dataSet2[dS2Index]) {

                ++dS1Index;
                ++dS2Index;

            }
            else
            {
                //match range ended
                break;
            }

        }

        //set range ends
        result->newAlignmentRange->inDataSet1.end = dS1Index;
        result->newAlignmentRange->inDataSet2.end = dS2Index;

        //set the next search ranges to the remainder of these search ranges
        result->dataSet1NextRange.start = dS1Index;
        result->dataSet1NextRange.end   = dataSet1Range.end;

        result->dataSet2NextRange.start = dS2Index;
        result->dataSet2NextRange.end   = dataSet2Range.end;


        return result;

    }
    else if (    (action == searchAction::advanceDataSet1PointerUntilAlignmentRange)
              || (action == searchAction::advanceDataSet2PointerUntilAlignmentRange) )
    {

        if (    (0 == dataSet1Range.count())
             || (0 == dataSet2Range.count()) ) {

            return nullptr;
        }

        std::unique_ptr<rangeMatch> alignmentRange;
        {
            if (action == searchAction::advanceDataSet1PointerUntilAlignmentRange) {

                alignmentRange = offsetMetrics::getNextAlignmentRange(dataSet1, dataSet2, dataSet1Range.start, dataSet1Range, dataSet2Range);

            } else {   // searchAction::advanceDataSet2PointerUntilAlignmentRange

                //swap dataSet references to step through dataSet2 instead of dataSet1
                alignmentRange = offsetMetrics::getNextAlignmentRange(dataSet2, dataSet1, dataSet2Range.start, dataSet2Range, dataSet1Range);

                //swap result ranges (so file1 refers to dataSet1, rather than reverse)
                rangeMatch* swapped = new rangeMatch(alignmentRange->startIndexInFile2,
                                                     alignmentRange->startIndexInFile1,
                                                     alignmentRange->byteCount);
                alignmentRange.reset(swapped);
            }
        }

        if (nullptr == alignmentRange) {
            //no alignmentRange found
            return nullptr;
        }

        result->dataSet1NextRange = dataSet1Range;
        result->dataSet2NextRange = dataSet2Range;

        ASSERT(noSumOverflow(alignmentRange->startIndexInFile1, alignmentRange->byteCount));
        result->dataSet1NextRange.start = alignmentRange->startIndexInFile1 + alignmentRange->byteCount;

        ASSERT(noSumOverflow(alignmentRange->startIndexInFile2, alignmentRange->byteCount));
        result->dataSet2NextRange.start = alignmentRange->startIndexInFile2 + alignmentRange->byteCount;

        result->newAlignmentRange = alignmentRange::fromRangeMatch(*alignmentRange);

        return result;
    }
    else
    {
        //unhandled searchProcessing::searchAction
        FAIL();
        return nullptr;
    }


}
