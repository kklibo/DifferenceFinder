#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <map>
#include <memory>

#include "byteRange.h"

class utilities
{
public:
    utilities() = delete;   //static functions only

    static unsigned int findStrongestRepetitionPeriod (
            const std::vector<unsigned char>& data,
            const byteRange inThisRange );

    static
    std::unique_ptr<std::vector<unsigned char>>
    createOffsetByteMap (
            const std::vector<unsigned char>& data,
            const byteRange inThisRange );

    static
    std::unique_ptr<std::vector<unsigned char>>
    createCrossFileOffsetByteMap (
            const std::vector<unsigned char>& source,
            const byteRange sourceRange,
            const std::vector<unsigned char>& target,
            const byteRange targetRange,
            const bool runBackwards );
};

#endif // UTILITIES_H
