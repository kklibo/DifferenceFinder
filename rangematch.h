#ifndef RANGEMATCH_H
#define RANGEMATCH_H

#include "byteRange.h"
#include "defensivecoding.h"

/*
represents a pair of matched byte ranges across 2 files
*/

class rangeMatch
{
public:
    explicit rangeMatch(const unsigned int startIndexInFile1,
                        const unsigned int startIndexInFile2,
                        const unsigned int byteCount);

    //these return the next index past the last index included in the range match in each file
    unsigned int getEndInFile1() const;
    unsigned int getEndInFile2() const;

    const unsigned int startIndexInFile1;
    const unsigned int startIndexInFile2;
    const unsigned int byteCount;

};

#endif // RANGEMATCH_H
