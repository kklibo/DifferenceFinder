#include "comparison.h"

/*static*/ std::atomic_bool comparison::m_abort{false};

/*static*/ unsigned int comparison::findLargestMatchingBlocks(  const std::vector<unsigned char>&   data1,
                                                                const std::vector<unsigned char>&   data2,
                                                                const std::multiset<indexRange>&    data1SkipRanges,
                                                                const std::multiset<indexRange>&    data2SkipRanges,
                                                                      std::multiset<blockMatchSet>& matches )
{
/*
    This finds the largest set of matching blocks occurring in both data1 and data2
    (or multiple sets, if there is a multi-way tie for largest).

    Bytes in the skip ranges will be ignored by this search;
    skip ranges are used to exclude blocks which have already been matched.

    Returns: the size (in bytes) of the largest matching blocks in data1 and data2.

    for current n: calculate rolling hash values across data1 and data2
        if there are no matches, decrease n
        if there are matches, increase n
        increase/decrease amounts are set for binary search
*/


    //the upper bound in the search (inclusive)
    unsigned int upperBound;
    //
    // the initial upper bound value will be the largest possible match that the search could return,
    //  which is the largest unbroken (by skip ranges) range that appears in both data1 and data2
    //
    //  if this happens to be the first iteration in a file comparison
    //   (i.e., skip ranges are empty because no blocks have been selected for matches yet),
    //   the initial upper bound will always be the minimum of the 2 data sets' sizes
    //
    {
        ASSERT_LE_UINT_MAX(data1.size());
        ASSERT_LE_UINT_MAX(data2.size());
        unsigned int data1Size = static_cast<unsigned int>(data1.size());
        unsigned int data2Size = static_cast<unsigned int>(data2.size());

        unsigned int largestGap_data1 = indexRange::findSizeOfLargestEmptySpace( indexRange(0, data1Size), data1SkipRanges );
        unsigned int largestGap_data2 = indexRange::findSizeOfLargestEmptySpace( indexRange(0, data2Size), data2SkipRanges );
        upperBound = std::min( largestGap_data1, largestGap_data2 );
    }

    //the lower bound in the search (inclusive)
    //  initial value is zero (the data sets might have no bytes in common)
    unsigned int lowerBound = 0;

    while (1) {

        if (0 == upperBound) {
            //no matches (at least one data set has zero length, or data sets have no bytes in common)
            return 0;
        }

        //current block size
        unsigned int blockSize = (upperBound+lowerBound+1)/2;  //average the current search bounds, rounding up
        //(rounding up prevents infinite loop from unchanging blockSize when upper and lower bounds are 1 apart)

        if (m_abort) {
            return 0;   //comparison has been aborted, just return immediately (results will be marked as aborted)
        }

        bool result;
        if (blockSize == upperBound){
            //we are about to check the largest remaining possible block size:
            //if any are found, they are the largest matching blocks between data1 and data2
            result = blockMatchSearch(blockSize, data1, data2, data1SkipRanges, data2SkipRanges, &matches);

            //remove some results (if necessary) to ensure that no matched block overlaps any other
            comparison::chooseValidMatchSets(matches);

            if (result) {
                ASSERT(0 < matches.size());
                return blockSize;
            }

        }
        else
        {
            result = blockMatchSearch(blockSize, data1, data2, data1SkipRanges, data2SkipRanges);
        }

        if (result) {
            //there's a match at this length, move the lower bound up to it
            lowerBound = blockSize;
        }
        else {
            //there's no match at this length, move the upper bound down below it
            upperBound = blockSize - 1;
        }

    }

}

/*
search for matching blocks between these data sets

if resultMatches is nullptr, results won't being returned, so
 the function will return true as soon as any block match between the 2 sets is found

*/
/*static*/ bool comparison::blockMatchSearch(   const unsigned int                  blockLength,
                                                const std::vector<unsigned char>&   data1,
                                                const std::vector<unsigned char>&   data2,
                                                const std::multiset<indexRange>&    data1SkipRanges,
                                                const std::multiset<indexRange>&    data2SkipRanges,
                                                      std::multiset<blockMatchSet>* resultMatches /*= nullptr*/ )
{
    if (0 == blockLength) {
        return false;
    }

    unsigned int spuriousHashCollisions = 0;
    auto reportSpuriousHashCollisions = MakeScopeExit(

        [&spuriousHashCollisions]() {
            if (!spuriousHashCollisions) {
                return;
            }
            std::string outStr = "spurious hash collisions: " + std::to_string(spuriousHashCollisions);
            Log::strMessageLvl3(outStr);
    } );


    //quickly traversable storage
    std::vector<unsigned int> hashes1;


    struct HashIndexPair {
        unsigned int hash;
        unsigned int index;
        HashIndexPair(unsigned int hash_, unsigned int index_)
            :   hash(hash_), index(index_)
        {
        }

        bool operator<(const HashIndexPair& rhs) const {
            return hash < rhs.hash;
        }
    };

    //quickly searchable storage
    //(not a vector, so we need to record the index in a HashIndexPair)
    std::multiset<HashIndexPair> hashes2;


    auto isBlockSkipped = [&blockLength](const unsigned int startIndex, const std::multiset<indexRange>& skipRanges) {

        //if block overlaps a indexRange in skipRange, then this block is skipped
        ASSERT(        noSumOverflow(startIndex,blockLength));
        indexRange block(startIndex, startIndex+blockLength);

        for (auto& skipRange : skipRanges) {
            if (block.overlaps(skipRange)) {
                return true;
            }
        }

        //if this is reached, block doesn't overlap any indexRanges in skipRanges
        return false;

    };


    auto addToHashes1 = [&hashes1](unsigned int hashValue, unsigned int /*not used*/){
         hashes1.push_back(hashValue);
    };

    auto addToHashes2 = [&hashes2](unsigned int hashValue, unsigned int index){
         hashes2.insert(HashIndexPair(hashValue, index));
    };

    auto getAllHashes = [blockLength](const std::vector<unsigned char>& data, std::function<void(unsigned int, unsigned int)> storeHashValue)
    {
        if (data.size() < blockLength) {return;}    //if there isn't enough for a full block, just return

        unsigned int index = 0;
        unsigned int hashValue;

        buzhash hasher(blockLength);

        //preload the hasher so the next byte completes the first block
        while (index < blockLength-1) {
                  unsigned char c = data[index++];
                  hashValue = hasher.hashByte(c);
        }

        while(index < data.size()) {
            unsigned char c = data[index];
            hashValue = hasher.hashByte(c);

            unsigned int blockStartIndex = index + 1 - blockLength;
            storeHashValue(hashValue, blockStartIndex);
            index++;
        }
    };

    auto blocksAreBytewiseEqual = [&blockLength](   const unsigned int block1StartIndex, const std::vector<unsigned char>& data1,
                                                    const unsigned int block2StartIndex, const std::vector<unsigned char>& data2) -> bool
    {
        for (unsigned int i = 0; i < blockLength; ++i) {
            if (    data1[block1StartIndex + i]
                 != data2[block2StartIndex + i] )
            {
                return false;
            }
        }
        return true;
    };

    //adds a block to an existing blockMatchSet, if there is one
    //returns false if this index/dataset's byte contents are not already in a blockMatchSet
    auto addToExistingBlockMatchSet = [&resultMatches, &blockLength, &data1, &data2, &blocksAreBytewiseEqual, &spuriousHashCollisions]
                                      (const unsigned int hash, const unsigned int startIndex, const whichDataSet&& source) -> bool
    {
        //select source data set to refer to
        const std::vector<unsigned char> *sourceDataSet = nullptr;
        if (whichDataSet::first == source) {
            sourceDataSet = &data1;
        }
        else if (whichDataSet::second == source) {
            sourceDataSet = &data2;
        }
        else {
            FAIL();
        }

        //find all blockMatchSets with the hash we're looking for
        auto matchRange = resultMatches->equal_range(blockMatchSet(hash,0,0,0));

        //iterate through them
        auto iter = matchRange.first;
        for ( ; iter != matchRange.second; ++iter ) {

            //get a reference index in data set 1 as a source of the byte contents represented by this blockMatchSet
            ASSERT(1 <= iter->data1_BlockStartIndices.size());
            unsigned int referenceIndex = iter->data1_BlockStartIndices[0];

            //see if the byte contents of this blockMatchSet actually match the block we're trying to add (i.e., not a hash collision)
            if (blocksAreBytewiseEqual(referenceIndex, data1, startIndex, *sourceDataSet)) {

                //match found, add this block to the matching blockMatchSet

                const std::vector<unsigned int>* addToThisIndexList = nullptr;

                if (whichDataSet::first == source) {
                    addToThisIndexList = &iter->data1_BlockStartIndices;
                }
                else if (whichDataSet::second == source) {
                    addToThisIndexList = &iter->data2_BlockStartIndices;
                }
                else {
                    FAIL();
                }

                //why is breaking const ok here?:
                //const-ness of std::multiset elements preserves the sorted-ness of the container
                //  changes are intended to be done by removing the element and reinserting a new one,
                //  so it is sorted into the right place for its new contents
                //(if there's another reason, this may not be ok)
                //blockMatchSet's < operator only uses its hash value, so changing other members shouldn't break sorted-ness

                auto tmp = const_cast<std::vector<unsigned int>*>(addToThisIndexList);
                tmp->emplace_back(startIndex);

                //we found a match and stored this block in it, stop searching
                break;
            }
            else {
                //the blocks didn't match each other: a spurious hash collision occurred
                ++spuriousHashCollisions;
            }
        }

        //if the iterator isn't at the end of matchRange, the current block has been assigned to a pre-existing blockMatchSet
        if (iter != matchRange.second) {
            return true;
        }

        return false;
    };



    bool matchFound = false;    //this will be set to true if we find a match (for return value)

    getAllHashes(data1, addToHashes1);
    getAllHashes(data2, addToHashes2);

    //loop through all the hashes of blocks from data set 1
    //(they're stored in order, so the index in hashes1[] is also the start index of the block in data1)
    for (unsigned int data1BlockStartIndex = 0; data1BlockStartIndex < hashes1.size(); ++data1BlockStartIndex) {

        if (isBlockSkipped(data1BlockStartIndex, data1SkipRanges)) {
            //if this block is to be skipped (i.e., would overlap previously completed match results), skip it
            continue;
        }

        unsigned int data1BlockHash = hashes1[data1BlockStartIndex];

        //if resultMatches exists, we're returning full match results instead of a quick bool return
        //if not, we return true on the first match, so we can skip this
        if (resultMatches) {
            //check if the byte content from this data set 1 block is already in a blockMatchSet

            //add this data set 1 block to an existing blockMatchSet that matches its byte contents (if there is one)
            // (if so, its matches in data set 2, if any, are already there)
            if (addToExistingBlockMatchSet(data1BlockHash, data1BlockStartIndex, whichDataSet::first))
            {
                //if this block was added to an existing group, skip to next block
                //this prevents blockMatchSets from being created for byte content that is already represented by a blockMatchSet
                continue;
            }
        }

        //get all the blocks in data set 2 with hashes equal to the current data set 1 block
        auto matchRange = hashes2.equal_range(HashIndexPair(data1BlockHash,0));

        //iterate through them and make sure they actually match (i.e., not a hash collision)
        for (auto iter = matchRange.first; iter != matchRange.second; ++iter ) {

            unsigned int data2BlockStartIndex = iter->index;

            if (isBlockSkipped(data2BlockStartIndex, data2SkipRanges)) {
                //if this block is to be skipped (i.e., would overlap previously completed match results), skip it
                continue;
            }

            if (blocksAreBytewiseEqual(data1BlockStartIndex, data1, data2BlockStartIndex, data2)){
                //match found
                if(!resultMatches) {
                    return true;    //if no output storage is provided by the caller, just return the result
                }

                //add this block to a blockMatchSet:
                //add this block to an existing blockMatchSet that matches its byte contents (if there is one)
                if (!addToExistingBlockMatchSet(data1BlockHash, data2BlockStartIndex, whichDataSet::second)) {
                    //if not, make a new blockMatchSet and add this block pair
                    resultMatches->emplace(data1BlockHash, blockLength, data1BlockStartIndex, data2BlockStartIndex);
                }
                matchFound = true;  //update return value to reflect successful match
            }
            else {
                //the blocks didn't match each other: a spurious hash collision occurred
                ++spuriousHashCollisions;
            }
        }
    }

    if (resultMatches) {
        //return true if a blockMatchSet was added
        return matchFound;
    }

    //if this is reached, no matches were found
    return false;
}

/*static*/ void comparison::chooseValidMatchSet( blockMatchSet& match,
                                                 const std::vector<unsigned int>& alreadyChosen1,
                                                 const std::vector<unsigned int>& alreadyChosen2 )
{
    //called with blockMatchSet cast to non-const: don't modify blockMatchSet::hash or multiset ordering will be disrupted

    //this function assumes that the blockMatchSet index lists are sorted in increasing order
    ASSERT(indexRange::isNonDecreasing(match.data1_BlockStartIndices));
    ASSERT(indexRange::isNonDecreasing(match.data2_BlockStartIndices));

    //step forward through the index lists, accepting the first block and then all future non-overlapping blocks (greedy algorithm)
    const unsigned int blockLength = match.blockSize;
    auto makeValidList = [&blockLength](const std::vector<unsigned int>& indices,
                                              std::vector<unsigned int>& validIndices,
                                        const std::vector<unsigned int>& alreadyChosen ) {

        for (unsigned int index : indices) {

            //if this block would overlap a block already validated in another blockMatchSet, skip it
            ASSERT(    noSumOverflow( index,blockLength));
            indexRange current(index, index+blockLength);
            if (current.overlapsAnyIn(alreadyChosen, blockLength)) {
                continue;
            }

            if ( 0 == validIndices.size() ) {
                validIndices.push_back(index);  //just add the first index as valid
            }
            else {
                //compare the current block to the last validated block
                unsigned int lastValidStart = validIndices.back();
                ASSERT(               noSumOverflow( lastValidStart,blockLength));
                indexRange lastValid(lastValidStart, lastValidStart+blockLength);

                //add the current block only if it doesn't overlap the validated block
                if ( !current.overlaps(lastValid) ) {
                    validIndices.push_back(index);
                }
            }
        }
    };

    std::vector<unsigned int> validated_data1_BlockStartIndices;
    std::vector<unsigned int> validated_data2_BlockStartIndices;

    makeValidList(match.data1_BlockStartIndices, validated_data1_BlockStartIndices, alreadyChosen1);
    makeValidList(match.data2_BlockStartIndices, validated_data2_BlockStartIndices, alreadyChosen2);


    //truncate the longer list to the shorter list's length
    unsigned long count = std::min( validated_data1_BlockStartIndices.size(),
                                    validated_data2_BlockStartIndices.size());

    validated_data1_BlockStartIndices.resize(count);
    validated_data2_BlockStartIndices.resize(count);
    //these index lists should now point to equal numbers of non-overlapping matching blocks

    //move validated index lists into match
    match.data1_BlockStartIndices = std::move(validated_data1_BlockStartIndices);
    match.data2_BlockStartIndices = std::move(validated_data2_BlockStartIndices);

}

/*static*/ void comparison::chooseValidMatchSets( std::multiset<blockMatchSet>& matches ) {

    //lists of already chosen blocks from previous iterations
    // (to ensure that valid match sets in matches are chosen without overlapping each other)
    std::vector<unsigned int> alreadyChosen1;
    std::vector<unsigned int> alreadyChosen2;

    auto appendVector = [](std::vector<unsigned int> appendThis, std::vector<unsigned int> toThis){
        toThis.reserve(toThis.size() + appendThis.size());
        toThis.insert(toThis.end(), appendThis.begin(), appendThis.end());
    };

    //for (const blockMatchSet& match : matches) {
    for (std::multiset<blockMatchSet>::iterator match = matches.begin(); match != matches.end(); ++match) {
        //casting to non-const: don't modify blockMatchSet::hash or multiset ordering will be disrupted
        chooseValidMatchSet(const_cast<blockMatchSet&>(*match), alreadyChosen1, alreadyChosen2);

        //remove match if empty
        if (    ( 0 == match->data1_BlockStartIndices.size())
             && ( 0 == match->data2_BlockStartIndices.size()) ) {
            match = matches.erase(match);
        }
        else {
            //record chosen blocks
            appendVector(match->data1_BlockStartIndices, alreadyChosen1);
            appendVector(match->data2_BlockStartIndices, alreadyChosen2);
        }
    }
}

/*static*/ void comparison::addMatchesToSkipRanges(  const std::multiset<blockMatchSet>& matches,
                                                           std::multiset<indexRange>&     data1SkipRanges,
                                                           std::multiset<indexRange>&     data2SkipRanges )
{
    for (const blockMatchSet& match : matches) {

        for (auto& index : match.data1_BlockStartIndices) {
            ASSERT(noSumOverflow(          index,match.blockSize));
            data1SkipRanges.emplace(index, index+match.blockSize);
        }

        for (auto& index : match.data2_BlockStartIndices) {
            ASSERT(noSumOverflow(          index,match.blockSize));
            data2SkipRanges.emplace(index, index+match.blockSize);
        }
    }
}

/*static*/ std::unique_ptr<std::list<indexRange>> comparison::findUnmatchedBlocks(  const indexRange& fillThisRange,
                                                                                    const std::multiset<blockMatchSet>& matches,
                                                                                    const whichDataSet which )
{
    std::list<indexRange> allBlocks;

    for (auto& match : matches) {

        const std::vector<unsigned int>& startIndices =
                whichDataSet::first == which
                ? match.data1_BlockStartIndices
                : match.data2_BlockStartIndices;

        for (auto& start : startIndices) {
            ASSERT(noSumOverflow(         start,match.blockSize));
            allBlocks.emplace_back(start, start+match.blockSize);
        }
    }

    allBlocks.sort();

    if(!indexRange::isNonDecreasingAndNonOverlapping(allBlocks)) {
        LOG.Warning("findUnmatchedBlocks block corruption");
    }

    //find spaces between blocks
    std::unique_ptr<std::list<indexRange>> copiesOfAddedBlocks ( new std::list<indexRange>() );
    indexRange::fillEmptySpaces(fillThisRange, allBlocks, *copiesOfAddedBlocks);

    //confirm partition
    if( !indexRange::isExactAscendingPartition(fillThisRange, allBlocks)) {
        LOG.Warning("findUnmatchedBlocks partition failure");
    }

    return copiesOfAddedBlocks;
}

/*static*/ std::unique_ptr<comparison::results> comparison::doCompare(  const std::vector<unsigned char>& data1,
                                                                        const std::vector<unsigned char>& data2 )
{
    m_abort = false; //clear abort flag

    auto Results = std::unique_ptr<comparison::results>( new comparison::results );

    if (!data1.size() || !data2.size()) {
        Results->internalError = true;
        return Results;
    }

    std::multiset<indexRange> data1SkipRanges;
    std::multiset<indexRange> data2SkipRanges;


    unsigned int largest = 1;
stopwatch sw;
sw.recordTime();
    do {
        std::multiset<blockMatchSet> matches; //matches for this iteration (will all have the same block size)

        largest = comparison::findLargestMatchingBlocks(data1, data2, data1SkipRanges, data2SkipRanges, matches);
        LOG.Debug(QString("Largest Matching Block Size: %1").arg(largest));

        if (m_abort) {
            //if the comparison is aborted while the above call to findLargestMatchingBlocks is running,
            // it will immediately return 0 (probably incorrectly).
            //this will mark the results as aborted and stop the algorithm
            Results->aborted = true;
            return Results;
        }

        comparison::addMatchesToSkipRanges(matches, data1SkipRanges, data2SkipRanges);

        if (!indexRange::isNonOverlapping(data1SkipRanges)) {
            LOG.Warning("data1SkipRanges");
        }
        if (!indexRange::isNonOverlapping(data2SkipRanges)) {
            LOG.Warning("data2SkipRanges");
        }

        //add to main match list
        Results->matches.insert(matches.begin(), matches.end());
sw.recordTime("finished largest " + std::to_string(largest));
    } while (largest > 0);

    ASSERT_LE_UINT_MAX(data1.size());
    unsigned int data1Size = static_cast<unsigned int>(data1.size());

    indexRange data1_FullRange (0, data1Size);
    Results->data1_unmatchedBlocks.swap(*comparison::findUnmatchedBlocks(data1_FullRange, Results->matches, comparison::whichDataSet::first));

    ASSERT_LE_UINT_MAX(data2.size());
    unsigned int data2Size = static_cast<unsigned int>(data2.size());

    indexRange data2_FullRange (0, data2Size);
    Results->data2_unmatchedBlocks.swap(*comparison::findUnmatchedBlocks(data2_FullRange, Results->matches, comparison::whichDataSet::second));

sw.recordTime("found unmatched blocks");
sw.reportTimes(&Log::strMessageLvl1);

    return Results;
}

void comparison::abort()
{
    m_abort = true;
}
