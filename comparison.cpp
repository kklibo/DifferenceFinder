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
    This returns the size (in bytes) of the largest contiguous block of bytes in data1 and data2

    for current n: calculate rolling hash values across data1 and data2
        if there are no matches, decrease n
        if there are matches, increase n
        increase/decrease for binary search

        when largest hash match is reached, confirm match with byte comparison
            if it isn't actually a match, reset lower bound to 0 and start new binary search
*/

    //the upper bound in the search (inclusive)
    //  this never increases, because different hashes always imply different data
    //  initial value is the minimum of the 2 data sets' sizes (the largest possible matching block size)
    unsigned int upperBound = std::min(data1.size(), data2.size());

    //the lower bound in the search (inclusive)
    //  this could decrease, because a hash match could be caused by a collision
    //  initial value is zero (the data sets might have no bytes in common)
    unsigned int lowerBound = 0;

    while (1) {

        if (0 == upperBound) {
            //no matches (at least one data set has zero length, or data sets have no bytes in common)
            return 0;
        }

        if (upperBound != lowerBound)   //still searching
        {
            //current n-gram length
            unsigned int n = (upperBound+lowerBound+1)/2;  //average the current search bounds, rounding up
            //(rounding up prevents infinite loop from unchanging n when upper and lower bounds are 1 apart)

            if (blockHashMatchExists(n, data1, data2)) {
                //there's a match at this length, move the lower bound up to it
                lowerBound = n;
            }
            else {
                //there's no match at this length, move the upper bound down below it
                upperBound = n - 1;
            }

        }
        else    //hash-based search has resolved a possible match
        {
            //test actual block content, not just hashes
            if (blockMatchExists(upperBound, data1, data2)) {
                //the content matches, and all longer hashes had no matches, we're done
                return upperBound;
            }
            else {
                //hash match was due to a hash collision, not a content match:
                //  restart the search below this
                upperBound -= 1;
                lowerBound = 0;
            }

        }

    }

}

bool comparison::blockMatchSearch(unsigned int blockLength, std::vector<unsigned char>& data1, std::vector<unsigned char>& data2, bool justCompareHashes)
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
    std::set<HashIndexPair> hashes2;


    auto addToHashes1 = [&hashes1](unsigned int hashValue, unsigned int){
         hashes1.push_back(hashValue);
    };

    auto addToHashes2 = [&hashes2](unsigned int hashValue, unsigned int index){
         hashes2.insert(HashIndexPair(hashValue, index));
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

    auto blocksAreBytewiseEqual = [&blockLength, &data1, &data2](unsigned int block1StartIndex, unsigned int block2StartIndex) -> bool
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

    for (unsigned int hashIndex = 0; hashIndex < hashes1.size(); ++hashIndex) {
        auto& hashValue = hashes1[hashIndex];

        auto matchRange = hashes2.equal_range(HashIndexPair(hashValue,0));

        for (auto iter = matchRange.first; iter != hashes2.end() && iter != matchRange.second; ++iter ) {

            if (justCompareHashes) {
                return true;
            }
            else {
                unsigned int block1StartIndex = hashIndex;
                unsigned int block2StartIndex = iter->index;
                if (blocksAreBytewiseEqual(block1StartIndex, block2StartIndex)){
                    return true;
                }
            }
        }
    }

    return false;
}

bool comparison::blockHashMatchExists(unsigned int blockLength, std::vector<unsigned char>& data1, std::vector<unsigned char>& data2)
{
    return blockMatchSearch(blockLength, data1, data2, true);
}

bool comparison::blockMatchExists(unsigned int blockLength, std::vector<unsigned char>& data1, std::vector<unsigned char>& data2)
{
    return blockMatchSearch(blockLength, data1, data2, false);
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

