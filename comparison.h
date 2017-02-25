#ifndef COMPARISON_H
#define COMPARISON_H

#include <vector>
#include <queue>
#include <set>
#include <memory>
#include <utility>
#include "rollinghashcpp/cyclichash.h"
#include "rollinghashcpp/rabinkarphash.h"
#include "rollinghashcpp/generalhash.h"


#include <iostream>

class comparison
{
public:
    comparison();

    static void rollingHashTest();
    void rollingHashTest2();

    unique_ptr<std::vector<unsigned int>> getRollingHashValues(std::vector<unsigned char>& data);

    unsigned int findLargestMatchingBlock(std::vector<unsigned char>& data1, std::vector<unsigned char>& data2);
    bool blockHashMatchExists   (unsigned int blockLength, std::vector<unsigned char>& data1, std::vector<unsigned char>& data2);
    bool blockMatchExists       (unsigned int blockLength, std::vector<unsigned char>& data1, std::vector<unsigned char>& data2);

private:

    bool blockMatchSearch(unsigned int blockLength, std::vector<unsigned char>& data1, std::vector<unsigned char>& data2, bool justCompareHashes);

    void createNewHasher(unsigned int n, unsigned int hashBits);
    unsigned int getNextRollingHashValue(unsigned char nextByte);
    std::shared_ptr<std::queue<unsigned char>> toRemove;
    std::shared_ptr<CyclicHash<>> hasher;

};

#endif // COMPARISON_H
