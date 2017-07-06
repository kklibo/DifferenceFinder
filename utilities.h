#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <map>
#include <memory>

#include "indexrange.h"

class utilities
{
public:
    utilities() = delete;   //static functions only

    static unsigned int findStrongestRepetitionPeriod (
            const std::vector<unsigned char>& data,
            const indexRange inThisRange );

    static
    std::unique_ptr<std::vector<unsigned char>>
    createOffsetByteMap (
            const std::vector<unsigned char>& data,
            const indexRange inThisRange );

    static
    std::unique_ptr<std::vector<unsigned char>>
    createCrossFileOffsetByteMap (
            const std::vector<unsigned char>& source,
            const indexRange sourceRange,
            const std::vector<unsigned char>& target,
            const indexRange targetRange,
            const bool runBackwards );

    static
    unsigned int
    subtractClampToZero (
            const unsigned int& value,
            const unsigned int& subtractThis);

    static
    unsigned int
    addClampToMax(
            const unsigned int& value,
            const unsigned int& addThis);

    static
    unsigned int
    countMatchingIndices(
            const std::vector<unsigned char>& data1,
            const std::vector<unsigned char>& data2,
            const indexRange& data1Subset,
            const indexRange& data2Subset);
};

#endif // UTILITIES_H
