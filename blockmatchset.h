#ifndef BLOCKMATCHSET_H
#define BLOCKMATCHSET_H

#include <memory>
#include <vector>

/*
records a set of identical matching byte blocks across 2 data sets,
possibly including multiple blocks in each set
*/

class blockMatchSet
{
public:
    blockMatchSet(unsigned int hash_, unsigned int blockSize_, unsigned int data1_initialBlockIndex, unsigned int data2_initialBlockIndex);

    unsigned int hash;
    unsigned int blockSize;
    std::vector<unsigned int>data1_BlockStartIndices;
    std::vector<unsigned int>data2_BlockStartIndices;

    bool operator < (const blockMatchSet& rhs) const;


};

#endif // BLOCKMATCHSET_H
