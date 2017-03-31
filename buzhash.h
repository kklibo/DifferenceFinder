#ifndef BUZHASH_H
#define BUZHASH_H

#include <vector>

#include "defensivecoding.h"

/*
    originally based on hash.go from
    https://github.com/silvasur/buzhash
*/


class buzhash
{

private:

    //hash function for a 1-byte input:
    // 256 32-bit values (from /dev/urandom)
    const static unsigned int bytehash[256];


    const unsigned int n;   //the hash window size, in bytes
    unsigned int state;     //the current hash value

    //circular buffer that records the contents of the hash window
    // (so that one can be removed each time a new one is added, FIFO-style)
    std::vector<unsigned char> buffer;
    unsigned int bufferPos; //current position in buffer
    bool bufferFilled;      //becomes true once the hash window buffer has been filled



public:

    buzhash(unsigned int hashWindowSize);

    //add a byte to the hash (returns the new hash value)
    unsigned int hashByte(const unsigned char b);

    void reset();   //resets (like a new object)

    unsigned int getHash();
    unsigned int getHashWindowSize();

};

#endif // BUZHASH_H
