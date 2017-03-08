#include "comparison.h"

comparison::comparison()
    :   toRemove(make_shared<std::queue<unsigned char>>()),
        hasher(nullptr)
{

}

unsigned int comparison::getNextRollingHashValue(unsigned char nextByte)
{
    toRemove->push(nextByte);
//    auto remove = toRemove->front();
 //   hasher->update(remove, nextByte);
    hasher->update(toRemove->front(), nextByte);
 //   std::cout << hex << (unsigned int)(toRemove->front()) << " " << hex << (unsigned int)nextByte; //<< std::endl;

    toRemove->pop();

 //   std::cout << "         " << hasher->hashvalue << std::endl;

    return hasher->hashvalue;
}

void comparison::createNewHasher(unsigned int n, unsigned int hashBits)
{
    hasher = make_shared<CyclicHash<>>(n,hashBits);
    toRemove = make_shared<std::queue<unsigned char>>();
    for (int i = 0; i < n; ++i) {
        //load n values (so the update calls have something to remove)
        hasher->eat(0);
        toRemove->push(0);
    }
}

unique_ptr<std::vector<unsigned int>> comparison::getRollingHashValues(std::vector<unsigned char>& data)
{
    if (nullptr == hasher) {
        return nullptr;
    }

    auto ret = unique_ptr<std::vector<unsigned int>>(new std::vector<unsigned int>());

    int n = hasher->n;  //length 'n' of the n-gram being hashed

    unsigned int hashValue;
    for(unsigned int i = 0; i < data.size(); ++i) {

        unsigned char c = data[i];
        hashValue = getNextRollingHashValue(c);

        if (i >= n-1){   //generate output when we have n values from this data set in the hash range
            ret->emplace_back(hashValue);
        }
    }

    return ret;
}

unsigned int comparison::findLargestMatchingBlocks( const std::vector<unsigned char>&   data1,
                                                    const std::vector<unsigned char>&   data2,
                                                    const std::multiset<byteRange>&     data1SkipRanges,
                                                    const std::multiset<byteRange>&     data2SkipRanges,
                                                          std::multiset<blockMatchSet>& matches )
{
/*
    This returns the size (in bytes) of the largest contiguous block(s) of bytes in data1 and data2

    for current n: calculate rolling hash values across data1 and data2
        if there are no matches, decrease n
        if there are matches, increase n
        increase/decrease amounts are set for binary search

*/

    //the upper bound in the search (inclusive)
    //  initial value is the minimum of the 2 data sets' sizes (the largest possible matching block size)
    unsigned int upperBound = std::min(data1.size(), data2.size());

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

        bool result;
        if (blockSize == upperBound){
            //we are about to check the largest remaining possible block size:
            //if any are found, they are the largest matching blocks between data1 and data2
            result = blockMatchSearch(blockSize, data1, data2, data1SkipRanges, data2SkipRanges, &matches);

            if (result) {
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
bool comparison::blockMatchSearch(  const unsigned int                  blockLength,
                                    const std::vector<unsigned char>&   data1,
                                    const std::vector<unsigned char>&   data2,
                                    const std::multiset<byteRange>&     data1SkipRanges,
                                    const std::multiset<byteRange>&     data2SkipRanges,
                                          std::multiset<blockMatchSet>* resultMatches /*= nullptr*/ )
{
    if (0 == blockLength) {
        return false;
    }


    unsigned int hashBits = 32;
    createNewHasher(blockLength, hashBits);

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


    //skipRanges are presumed to be non-overlapping
    auto isBlockSkipped = [&blockLength](const unsigned int startIndex, const std::multiset<byteRange>& skipRanges) {

        //if block overlaps a byteRange in skipRange, then this block is skipped
        byteRange block(startIndex, blockLength);

        auto equalRange = skipRanges.equal_range(block);

        if (equalRange.first != equalRange.second) {
            //equal range is non-zero: a byteRange starting at startIndex is in skipRanges (guaranteed overlap)
            return true;
        }

        //because we didn't just return, we know that equalRange.first and .second are the same

        if (equalRange.first != skipRanges.begin()) {
            //there is a byterange in skipRanges with a start index less than block's
            auto& previous = equalRange.first;
            --previous; //point to the previous byteRange

            if (block.overlaps(*previous)) {
                return true;
            }
        }

        if (equalRange.first != skipRanges.end()) {
            //there is a byterange in skipRanges with a start index greater than block's
            auto& next = equalRange.first;
            --next; //point to the next byteRange

            if (block.overlaps(*next)) {
                return true;
            }
        }

        //if this is reached, byteRanges adjacent to block in the sort order don't overlap it
        return false;

    };


    auto addToHashes1 = [&hashes1](unsigned int hashValue, unsigned int){
         hashes1.push_back(hashValue);
    //    std::cout << "h1 " << std::hex << hashValue << std::endl;
    };

    auto addToHashes2 = [&hashes2](unsigned int hashValue, unsigned int index){
         hashes2.insert(HashIndexPair(hashValue, index));
    //    std::cout << "h2 " << std::hex << hashValue << std::endl;
    };

    auto getAllHashes = [this, blockLength](const std::vector<unsigned char>& data, std::function<void(unsigned int, unsigned int)>storeHashValue)
    {
        if (data.size() < blockLength) {return;}    //if there isn't enough for a full block, just return

        unsigned int index = 0;
        unsigned int hashValue;
        //preload the hasher so the next byte completes the first block
        while (index < blockLength-1) {
                  unsigned char c = data[index++];
                  hashValue = getNextRollingHashValue(c);
        }

        while(index < data.size()) {
            unsigned char c = data[index];
            hashValue = getNextRollingHashValue(c);

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

    enum class whichDataSet{
        first,
        second
    };

    //adds a block to an existing blockMatchSet, if there is one
    //returns false if this index/dataset's byte contents are not already in a blockMatchSet
    auto addToExistingBlockMatchSet = [&resultMatches, &blockLength, &data1, &data2, &blocksAreBytewiseEqual]
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
        }

        //if the iterator isn't at the end of matchRange, the current block has been assigned to a pre-existing blockMatchSet
        if (iter != matchRange.second) {
            return true;
        }

        return false;
    };



    bool matchFound = false;    //this will be set to true if we find a match (for return value)

  //  std::cout << std::endl << "=== blockLength " << blockLength << " === data1 ===" << std::endl;
    getAllHashes(data1, addToHashes1);
  //  std::cout << std::endl << "=== blockLength " << blockLength << " === data2 ===" << std::endl;
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
        }
    }

    if (resultMatches) {
        //return true if a blockMatchSet was added
        return matchFound;
    }

    //if this is reached, no matches were found
    return false;
}

/*static*/ void comparison::chooseValidMatchSet( blockMatchSet& match ) {
    //called with blockMatchSet cast to non-const: don't modify blockMatchSet::hash or multiset ordering will be disrupted

    //this function assumes that the blockMatchSet index lists are sorted in increasing order

    //step forward through the index lists, accepting the first block and then all future non-overlapping blocks (greedy algorithm)
    const unsigned int blockLength = match.blockSize;
    auto makeValidList = [&blockLength](const std::vector<unsigned int>& indices, std::vector<unsigned int>& validIndices) {

        for (unsigned int index : indices) {
            if ( 0 == validIndices.size() ) {
                validIndices.push_back(index);  //just add the first index as valid
            }
            else {
                //compare the current block to the last validated block
                byteRange current   (index,                 blockLength);
                byteRange lastValid (validIndices.back(),   blockLength);

                //add the current block only if it doesn't overlap the validated block
                if ( !current.overlaps(lastValid) ) {
                    validIndices.push_back(index);
                }
            }
        }
    };

    std::vector<unsigned int> validated_data1_BlockStartIndices;
    std::vector<unsigned int> validated_data2_BlockStartIndices;

    makeValidList(match.data1_BlockStartIndices, validated_data1_BlockStartIndices);
    makeValidList(match.data2_BlockStartIndices, validated_data2_BlockStartIndices);


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

    for (const blockMatchSet& match : matches) {
        //casting to non-const: don't modify blockMatchSet::hash or multiset ordering will be disrupted
        chooseValidMatchSet(const_cast<blockMatchSet&>(match));
    }
}

/*static*/ void comparison::addMatchesToSkipRanges(  const std::multiset<blockMatchSet>& matches,
                                                std::multiset<byteRange>&     data1SkipRanges,
                                                std::multiset<byteRange>&     data2SkipRanges )
{
    for (const blockMatchSet& match : matches) {

        for (auto& index : match.data1_BlockStartIndices) {
            data1SkipRanges.emplace(index,match.blockSize);
        }

        for (auto& index : match.data2_BlockStartIndices) {
            data2SkipRanges.emplace(index,match.blockSize);
        }
    }
}


void comparison::rollingHashTest2()
{
    std::vector<unsigned char> data = {0,1,2,3,4,0,1,2,3,4};


    std::cout<<"rollingHashTest2" << std::endl;

    uint32 n = 5;
    int L = 9;

    createNewHasher(n,L);

    int index=0;
    unsigned int hashValue;
    for(uint32 k = 0; k<n;++k) {
              unsigned char c = data[index++]; ; // grab some character
              hashValue = getNextRollingHashValue(c);
    }

    for (int i = 0; i < 2; ++i) {

        std::cout<< " initial hashvalue: " << hex << hashValue << std::endl;

        while(index < data.size()) {

            unsigned char c = data[index];// points to the next character
            hashValue = getNextRollingHashValue(c);

           std::cout<< " hashvalue: 0x" << hex << hashValue << "  c: 0x" << hex << (uint32)c << std::endl;

           index++;
        }

        std::cout << std::endl;
        index = n;
    }

}


/*static*/ void comparison::rollingHashTest()
{
    std::vector<unsigned char> data = {0,1,2,3,4,0,1,2,3,4};


    std::cout<<"rollingHashTest" << std::endl;

    uint32 n = 5;
    int L = 9;

    //GeneralHash<> hf(n,L );
    //KarpRabinHash<> hf(n,L );
    CyclicHash<> hf(n,L );

    int index=0;
    for(uint32 k = 0; k<n;++k) {
              unsigned char c = data[index++]; ; // grab some character
              hf.eat(c); // feed it to the hasher
    }

    for (int i = 0; i < 2; ++i) {

/*
        for(uint32 k = 0; k<n;++k) {
                  unsigned char c = data[index++]; ; // grab some character
                  hf.eat(c); // feed it to the hasher
        }
*/
        std::cout<< " initial hashvalue: " << hex << hf.hashvalue << std::endl;

        while(index < data.size()) { // go over your string
           hf.hashvalue; // at all times, this contains the hash value
           unsigned char c = data[index];// points to the next character
           unsigned char out = data[index-n]; // character we want to forget

           hf.update(out,c); // update hash value

           std::cout<< " hashvalue: 0x" << hex << hf.hashvalue << "  c: 0x" << hex << (uint32)c << "  out: 0x" << hex << (uint32)out << std::endl;

           index++;
        }

        std::cout << std::endl;
        index = n;
    }

}

