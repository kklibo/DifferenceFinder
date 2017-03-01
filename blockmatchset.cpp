#include "blockmatchset.h"

blockMatchSet::blockMatchSet(unsigned int hash_, unsigned int blockSize_, unsigned int data1_initialBlockIndex, unsigned int data2_initialBlockIndex)
    :   hash(hash_),
        blockSize(blockSize_),
        data1_BlockStartIndices(),
        data2_BlockStartIndices()
{
    data1_BlockStartIndices.emplace_back(data1_initialBlockIndex);
    data2_BlockStartIndices.emplace_back(data2_initialBlockIndex);
}

bool blockMatchSet::operator < (const blockMatchSet& rhs) const
{
    return hash < rhs.hash;
}
