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

unsigned int comparison::findLargestMatchingBlock(std::vector<unsigned char>& data1, std::vector<unsigned char>& data2)
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

            result = blockMatchSearch(blockSize, data1, data2);

            if (result) {
                return blockSize;
            }

        }
        else
        {
            result = blockMatchSearch(blockSize, data1, data2);
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

if allMatches is nullptr, results won't being returned, so
 the function will return true as soon as any block match between the 2 sets is found

*/
bool comparison::blockMatchSearch(  unsigned int blockLength,
                        std::vector<unsigned char>& data1,
                        std::vector<unsigned char>& data2,
                        std::multiset<blockMatchSet>* allMatches /*= nullptr*/ )
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
    std::multiset<HashIndexPair> hashes2;


    auto addToHashes1 = [&hashes1](unsigned int hashValue, unsigned int){
         hashes1.push_back(hashValue);
        std::cout << "h1 " << std::hex << hashValue << std::endl;
    };

    auto addToHashes2 = [&hashes2](unsigned int hashValue, unsigned int index){
         hashes2.insert(HashIndexPair(hashValue, index));
        std::cout << "h2 " << std::hex << hashValue << std::endl;
    };

    auto getAllHashes = [this, blockLength](std::vector<unsigned char>& data, std::function<void(unsigned int, unsigned int)>storeHashValue)
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

    auto blocksAreBytewiseEqual = [&blockLength](unsigned int block1StartIndex, std::vector<unsigned char>& data1,
                                                 unsigned int block2StartIndex, std::vector<unsigned char>& data2) -> bool
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

  //  std::cout << std::endl << "=== blockLength " << blockLength << " === data1 ===" << std::endl;
    getAllHashes(data1, addToHashes1);
  //  std::cout << std::endl << "=== blockLength " << blockLength << " === data2 ===" << std::endl;
    getAllHashes(data2, addToHashes2);

    //loop through all the hashes of blocks from data set 1
    for (unsigned int hashIndex = 0; hashIndex < hashes1.size(); ++hashIndex) {
        auto& data1BlockHash = hashes1[hashIndex];

        //if allMatches exists, we're returning results instead of a quick bool return
        if (allMatches) {
            //check if the byte content from this data set 1 block is already in a blockMatchSet
            // if so, add it to that blockMatchSet (its matches in data set 2, if any, are already there)
            //this prevents blockMatchSets from being created for byte content that is already represented by a blockMatchSet
//std::multiset<blockMatchSet> test;
//auto matchRange2 = test.equal_range(blockMatchSet(data1BlockHash,blockLength));

            auto matchRange = allMatches->equal_range(blockMatchSet(data1BlockHash,0,0,0));
//std::pair<std::multiset<blockMatchSet>::iterator, std::multiset<blockMatchSet>::iterator> matchRange = allMatches->equal_range(blockMatchSet(data1BlockHash,blockLength));

            //iterate through them and make sure they actually match (i.e., not a hash collision)
            bool data1BlockAlreadyMatched = false;
            auto iter = matchRange.first;
            for ( ; iter != matchRange.second; ++iter ) {
//for (std::multiset<blockMatchSet>::iterator iter = matchRange.first; iter != matchRange.second; ++iter ) {

                unsigned int block1StartIndex = hashIndex;
/*check this*/  unsigned int block2StartIndex = iter->data1_BlockStartIndices[0];

                //   (data1 is correct, we are comparing 2 blocks in the first set)   *****
                if (blocksAreBytewiseEqual(block1StartIndex, data1, block2StartIndex, data1)){
                //                                                                    *****
                    //match found, add this block to the matching blockMatchSet

                    //why is breaking const ok here?:
                    //const-ness of std::multiset elements preserves the sorted-ness of the container
                    //  changes are to be done by removing the element and reinserting a new one,
                    //  so it is sorted into the right place for its new contents
                    //(if there's another reason, this may not be ok)
                    //blockMatchSet's < operator only uses its hash value, so changing other members shouldn't break sorted-ness
/*switch to auto?*/ std::vector<unsigned int>* tmp = const_cast<std::vector<unsigned int>*>(&iter->data1_BlockStartIndices);
                    tmp->emplace_back(block1StartIndex);

                    //we found a match and stored this block in it, stop searching
                    data1BlockAlreadyMatched = true;
                    break;
                }
            }

            //if the iterator isn't at the end of matchRange, the current block has been assigned to a pre-existing blockMatchSet
            //skip to the next block from data set 1
            if (iter != matchRange.second) {
                std::cout << "continue" << std::endl;
                continue;
            }
        }

        //get all the blocks in data set 2 with hashes equal to the current data set 1 block
        auto matchRange = hashes2.equal_range(HashIndexPair(data1BlockHash,0));

        //iterate through them and make sure they actually match (i.e., not a hash collision)
        for (auto iter = matchRange.first; iter != matchRange.second; ++iter ) {

            unsigned int block1StartIndex = hashIndex;
            unsigned int block2StartIndex = iter->index;
            if (blocksAreBytewiseEqual(block1StartIndex, data1, block2StartIndex, data2)){
                //match found
                if(!allMatches) {
                    return true;    //no output storage provided by caller, just return the result
                }

                //add this block to a blockMatchSet
/*possibly deduplicate code?, replace blank blockMatchSet construction method?*/
                auto matchRange2 = allMatches->equal_range(blockMatchSet(data1BlockHash,0,0,0));

                //iterate through them and make sure they actually match (i.e., not a hash collision)
                unsigned int index1 = hashIndex;
                unsigned int index2 = block2StartIndex;
                auto iter2 = matchRange2.first;
                for ( ; iter2 != matchRange2.second; ++iter2 ) {

                    if (blocksAreBytewiseEqual(index1, data1, index2, data2)){

                        // ***
    /*switch to auto?*/ std::vector<unsigned int>* tmp = const_cast<std::vector<unsigned int>*>(&iter2->data2_BlockStartIndices);
                        tmp->emplace_back(block2StartIndex);

                        //we found a match and stored this block in it, stop searching
                        break;
                    }

                }

                //if the iterator is at the end of matchRange2, the byte contents of this block aren't already in a blockMatchSet
                if (iter2 == matchRange2.second) {
                    //make a new blockMatchSet
                    allMatches->emplace(data1BlockHash, blockLength, index1, index2);
                }

            }
        }
    }

    return false;
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


void comparison::rollingHashTest()
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

