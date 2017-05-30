#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <map>

#include "byteRange.h"

class utilities
{
public:
    utilities() = delete;   //static functions only

    static unsigned int findStrongestRepetitionPeriod (
            const std::vector<unsigned char>& data,
            const byteRange inThisRange );

};

#endif // UTILITIES_H
