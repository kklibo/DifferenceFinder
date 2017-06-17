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
};

#endif // UTILITIES_H
