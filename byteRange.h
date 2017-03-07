#ifndef BYTERANGE_H
#define BYTERANGE_H

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

        return result;
    }
};

#endif // BYTERANGE_H
