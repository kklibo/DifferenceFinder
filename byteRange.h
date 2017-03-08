#ifndef BYTERANGE_H
#define BYTERANGE_H

#include <vector>
#include <algorithm>

/*
    specifies a range of bytes
*/

class byteRange {
public:
    unsigned int start; //the first index in the range
    unsigned int count; //the number of bytes

    byteRange(){
        start = 0;
        count = 0;
    }

    //creates a range of 1 byte at the start index
    byteRange(unsigned int start){
        this->start = start;
        count = 1;
    }

    byteRange(unsigned int start, unsigned int count){
        this->start = start;
        this->count = count;
    }

    //returns the index that is one past the end of this byteRange
    unsigned int end() const {
        return start + count;
    }

    bool overlaps(const byteRange& b) const {

        if ((0 == count) || (0 == b.count)) {
            return false;   //ranges w/ count 0 don't overlap anything
        }

        if(start < b.start) {
            return b.start < end();
        }
        else {
            return start < b.end();
        }
    }

    bool operator==(const byteRange& b) const {
        return (start == b.start) && (count == b.count);
    }
    bool operator!=(const byteRange& b) const {
        return !( *this == b );
    }
    bool operator< (const byteRange& b) const {
        return start < b.start;
    }

    //note: a byterange repeating the start index of a prior byteRange w/count zero
    //  will not cause this to return false (this shouldn't come up in normal use)
    template <typename T>
    static bool isNonDecreasingAndNonOverlapping(const T& byteRanges) {

        unsigned int minStart = 0;  //minimum start for subsequent acceptible byteRanges

        for (const byteRange& r : byteRanges) {

            if (r.start < minStart) {
                return false;   //the start of this range is before or inside a previous range
            }

            minStart = r.end(); //update minStart and continue stepping through the byteRanges

        }

        return true;    //no violations found
    }

    //if possible, call isNonDecreasingAndNonOverlapping instead:
    // this function copies and sorts the byteRanges
    //note: this function throws out byteRanges with count 0 (they can't overlap anything)
    template <typename T>
    static bool isNonOverlapping(const T& byteRanges) {

        std::vector<byteRange> sortCopy;
        for (byteRange b : byteRanges) {
            if (0 < b.count) {
                sortCopy.push_back(b);
            }
        }
        std::sort(sortCopy.begin(), sortCopy.end());

        return isNonDecreasingAndNonOverlapping(sortCopy);
    }


    static bool test() {

        bool result = true;
        byteRange a(20, 10);

        result = result && (    a.overlaps(a)                   );

        result = result && (  ! a.overlaps(byteRange(15, 5))    );
        result = result && (    a.overlaps(byteRange(20, 5))    );
        result = result && (    a.overlaps(byteRange(25, 5))    );
        result = result && (  ! a.overlaps(byteRange(30, 5))    );

        result = result && (  ! a.overlaps(byteRange(10,10))    );
        result = result && (    a.overlaps(byteRange(15,10))    );
        result = result && (    a.overlaps(byteRange(20,10))    );
        result = result && (    a.overlaps(byteRange(25,10))    );
        result = result && (  ! a.overlaps(byteRange(30,10))    );

        result = result && (  ! a.overlaps(byteRange( 0,20))    );
        result = result && (    a.overlaps(byteRange( 5,20))    );
        result = result && (    a.overlaps(byteRange(10,20))    );
        result = result && (    a.overlaps(byteRange(15,20))    );
        result = result && (    a.overlaps(byteRange(20,20))    );
        result = result && (    a.overlaps(byteRange(25,20))    );
        result = result && (  ! a.overlaps(byteRange(30,20))    );

        result = result && (  ! a.overlaps(byteRange(15, 0))    );
        result = result && (  ! a.overlaps(byteRange(20, 0))    );
        result = result && (  ! a.overlaps(byteRange(25, 0))    );
        result = result && (  ! a.overlaps(byteRange(30, 0))    );

        byteRange b(20, 0);

        result = result && (  ! b.overlaps(b)                   );
        result = result && (  ! b.overlaps(byteRange(10,10))    );
        result = result && (  ! b.overlaps(byteRange(15,10))    );
        result = result && (  ! b.overlaps(byteRange(20,10))    );

        std::vector<byteRange> r1 = { byteRange(10,10), byteRange(20,10), byteRange(35,0), byteRange(35,10) };
        result = result && (    isNonDecreasingAndNonOverlapping(r1)   );

        std::vector<byteRange> r2 = { byteRange(10,10), byteRange(35,0), byteRange(20,10), byteRange(35,10) };
        result = result && (  ! isNonDecreasingAndNonOverlapping(r2)   );

        std::vector<byteRange> r3 = { byteRange(40,10), byteRange(20,10), byteRange(30,0), byteRange(10,10) };
        result = result && (    isNonOverlapping(r3)   );

        std::vector<byteRange> r4 = { byteRange(40,10), byteRange(20,15), byteRange(30,0), byteRange(10,10) };
        result = result && (    isNonOverlapping(r4)   );

        std::vector<byteRange> r5 = { byteRange(40,10), byteRange(20,15), byteRange(30,1), byteRange(10,10) };
        result = result && (  ! isNonOverlapping(r5)   );

        return result;
    }
};

#endif // BYTERANGE_H
