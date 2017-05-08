#include "rangematch.h"

rangeMatch::rangeMatch(const unsigned int startIndexInFile1,
                       const unsigned int startIndexInFile2,
                       const unsigned int byteCount)
    :   startIndexInFile1(startIndexInFile1),
        startIndexInFile2(startIndexInFile2),
        byteCount(byteCount)
{
}

unsigned int rangeMatch::getEndInFile1() const
{
    return startIndexInFile1 + byteCount;
}

unsigned int rangeMatch::getEndInFile2() const
{
    return startIndexInFile2 + byteCount;
}
