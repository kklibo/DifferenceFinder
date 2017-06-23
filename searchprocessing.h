#ifndef SEARCHPROCESSING_H
#define SEARCHPROCESSING_H

#include <memory>

#include "indexrange.h"
#include "offsetmetrics.h"

#include "defensivecoding.h"

class searchProcessing
{
public:
    searchProcessing() = delete;    //static functions only

    enum class searchAction {
        none,
        advanceDataSet1PointerUntilMatchRange,
        advanceDataSet2PointerUntilMatchRange,
        advanceDataSet1PointerUntilAlignmentRange,
        advanceDataSet2PointerUntilAlignmentRange,
        advanceBothPointersBy1
    };


    struct alignmentRange {
        indexRange inDataSet1;
        indexRange inDataSet2;

        //needed?
        alignmentRange()
            : inDataSet1(),
              inDataSet2()
        {
        }

        static std::unique_ptr<alignmentRange> fromRangeMatch(const rangeMatch& r)
        {
            std::unique_ptr<alignmentRange> ret(new alignmentRange);
            ret->inDataSet1.start = r.startIndexInFile1;
            ASSERT(noSumOverflow(r.startIndexInFile1,r.byteCount));
            ret->inDataSet1.end = r.startIndexInFile1 + r.byteCount;

            ret->inDataSet2.start = r.startIndexInFile2;
            ASSERT(noSumOverflow(r.startIndexInFile2,r.byteCount));
            ret->inDataSet2.end = r.startIndexInFile2 + r.byteCount;
            return ret;
        }
    };

    //privatize members?
    struct searchState
    {
        std::unique_ptr<alignmentRange> newAlignmentRange;
        std::shared_ptr<searchState> parentState;
        searchAction actionTaken;

        indexRange dataSet1NextRange;
        indexRange dataSet2NextRange;

        //is this necessary?
        //  would creation w/ only default constructor always call byteRange default constructor?
        searchState()
            : newAlignmentRange(nullptr),
              parentState(nullptr),
              actionTaken(searchAction::none),
              dataSet1NextRange(),
              dataSet2NextRange()
        {
        }
    };


    static std::unique_ptr<searchState>
    doSearch (
        const std::vector<unsigned char>& dataSet1,
        const std::vector<unsigned char>& dataSet2,
        const std::shared_ptr<searchState> parentState,
        const searchAction action,
        const indexRange dataSet1Range,
        const indexRange dataSet2Range );

};

#endif // SEARCHPROCESSING_H
