#include "byteRange.h"
#include "gtestDefs.h"
#include <gtest.h>

TEST(byteRange, contains){
    //size zero
    byteRange a(10, 0);
    EXPECT_FALSE(   a.contains(0)                   );
    EXPECT_FALSE(   a.contains(9)                   );
    EXPECT_FALSE(   a.contains(10)                  );
    EXPECT_FALSE(   a.contains(11)                  );

    byteRange b(20, 10);

    EXPECT_FALSE(   b.contains(0)                   );
    EXPECT_TRUE (   b.contains(20)                  );
    EXPECT_TRUE (   b.contains(29)                  );
    EXPECT_FALSE(   b.contains(30)                  );
}

TEST(byteRange, overlaps){
    byteRange a(20, 10);

    //identity
    EXPECT_TRUE (   a.overlaps(a)                   );

    //smaller
    EXPECT_FALSE(   a.overlaps(byteRange(15, 5))    );
    EXPECT_TRUE (   a.overlaps(byteRange(20, 5))    );
    EXPECT_TRUE (   a.overlaps(byteRange(25, 5))    );
    EXPECT_FALSE(   a.overlaps(byteRange(30, 5))    );

    //same size
    EXPECT_FALSE(   a.overlaps(byteRange(10,10))    );
    EXPECT_TRUE (   a.overlaps(byteRange(15,10))    );
    EXPECT_TRUE (   a.overlaps(byteRange(20,10))    );
    EXPECT_TRUE (   a.overlaps(byteRange(25,10))    );
    EXPECT_FALSE(   a.overlaps(byteRange(30,10))    );

    //larger
    EXPECT_FALSE(   a.overlaps(byteRange( 0,20))    );
    EXPECT_TRUE (   a.overlaps(byteRange( 5,20))    );
    EXPECT_TRUE (   a.overlaps(byteRange(10,20))    );
    EXPECT_TRUE (   a.overlaps(byteRange(15,20))    );
    EXPECT_TRUE (   a.overlaps(byteRange(20,20))    );
    EXPECT_TRUE (   a.overlaps(byteRange(25,20))    );
    EXPECT_FALSE(   a.overlaps(byteRange(30,20))    );

    //zero size
    EXPECT_FALSE(   a.overlaps(byteRange(15, 0))    );
    EXPECT_FALSE(   a.overlaps(byteRange(20, 0))    );
    EXPECT_FALSE(   a.overlaps(byteRange(25, 0))    );
    EXPECT_FALSE(   a.overlaps(byteRange(30, 0))    );

    byteRange b(20, 0);

    EXPECT_FALSE(   b.overlaps(b)                   );
    EXPECT_FALSE(   b.overlaps(byteRange(10,10))    );
    EXPECT_FALSE(   b.overlaps(byteRange(15,10))    );
    EXPECT_FALSE(   b.overlaps(byteRange(20,10))    );
}

TEST(byteRange, getIntersection){
    {
        //same count
        byteRange a(20,10);
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange( 0,10))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange(10,10))  );
        EXPECT_EQ(  byteRange(20, 5), a.getIntersection(byteRange(15,10))  );
        EXPECT_EQ(  byteRange(20,10), a.getIntersection(byteRange(20,10))  );
        EXPECT_EQ(  byteRange(25, 5), a.getIntersection(byteRange(25,10))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange(30,10))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange(40,10))  );
    }
    {
        //larger
        byteRange a(30,10);
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange( 0,20))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange(10,20))  );
        EXPECT_EQ(  byteRange(30, 5), a.getIntersection(byteRange(15,20))  );
        EXPECT_EQ(  byteRange(30,10), a.getIntersection(byteRange(20,20))  );
        EXPECT_EQ(  byteRange(30,10), a.getIntersection(byteRange(25,20))  );
        EXPECT_EQ(  byteRange(30,10), a.getIntersection(byteRange(30,20))  );
        EXPECT_EQ(  byteRange(35, 5), a.getIntersection(byteRange(35,20))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange(40,20))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange(50,20))  );
    }
    {
        //smaller
        byteRange a(10,10);
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange( 0, 5))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange( 5, 5))  );
        EXPECT_EQ(  byteRange(10, 5), a.getIntersection(byteRange(10, 5))  );
        EXPECT_EQ(  byteRange(15, 5), a.getIntersection(byteRange(15, 5))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange(20, 5))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange(25, 5))  );
    }
    {
        //zero count
        byteRange a(10, 0);
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange( 0, 5))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange( 0,20))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange( 5, 5))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange( 5,10))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange(10, 5))  );
        EXPECT_EQ(  byteRange( 0, 0), a.getIntersection(byteRange(15, 5))  );
    }
}

TEST(byteRange, isNonDecreasingAndNonOverlapping){
    {
        std::vector<byteRange> ranges = { byteRange(10,10), byteRange(20,10), byteRange(35,0), byteRange(35,10) };
        EXPECT_TRUE (    byteRange::isNonDecreasingAndNonOverlapping(ranges)   );
    }
    {   //decreasing
        std::vector<byteRange> ranges = { byteRange(10,10), byteRange(35,0), byteRange(20,10), byteRange(35,10) };
        EXPECT_FALSE(    byteRange::isNonDecreasingAndNonOverlapping(ranges)   );
    }
    {
        std::vector<unsigned int> uints = {1,2,3};
        unsigned int blockSize = 0;
        EXPECT_TRUE (    byteRange::isNonDecreasingAndNonOverlapping(uints, blockSize) );
    }
    {
        std::vector<unsigned int> uints = {10,20,30,45};
        unsigned int blockSize = 10;
        EXPECT_TRUE (    byteRange::isNonDecreasingAndNonOverlapping(uints, blockSize) );
    }
    {   //decreasing
        std::list<unsigned int> uints = {10,20,45,30};
        unsigned int blockSize = 10;
        EXPECT_FALSE(    byteRange::isNonDecreasingAndNonOverlapping(uints, blockSize) );
    }
    {   //overlapping
        std::list<unsigned int> uints = {10,20,30,35};
        unsigned int blockSize = 10;
        EXPECT_FALSE(    byteRange::isNonDecreasingAndNonOverlapping(uints, blockSize) );
    }
}

TEST(byteRange, isNonOverlapping){
    {
        std::vector<unsigned int> uints = {1,2,3};
        unsigned int blockSize = 0;
        EXPECT_TRUE (    byteRange::isNonOverlapping(uints, blockSize) );
    }
    {
        std::vector<unsigned int> uints = {10,20,30,45};
        unsigned int blockSize = 10;
        EXPECT_TRUE (    byteRange::isNonOverlapping(uints, blockSize) );
    }
    {
        std::list<unsigned int> uints = {10,20,45,30};
        unsigned int blockSize = 10;
        EXPECT_TRUE (    byteRange::isNonOverlapping(uints, blockSize) );
    }
    {
        std::list<unsigned int> uints = {10,20,30,35};
        unsigned int blockSize = 10;
        EXPECT_FALSE(    byteRange::isNonOverlapping(uints, blockSize) );
    }
    {
        std::vector<byteRange> ranges = { byteRange(40,10), byteRange(20,10), byteRange(30,0), byteRange(10,10) };
        EXPECT_TRUE (    byteRange::isNonOverlapping(ranges)   );
    }
    {
        std::vector<byteRange> ranges = { byteRange(40,10), byteRange(20,15), byteRange(30,0), byteRange(10,10) };
        EXPECT_TRUE (    byteRange::isNonOverlapping(ranges)   );
    }
    {
        std::vector<byteRange> ranges = { byteRange(40,10), byteRange(20,15), byteRange(30,1), byteRange(10,10) };
        EXPECT_FALSE(    byteRange::isNonOverlapping(ranges)   );
    }
}

TEST(byteRange, isNonDecreasing){
    {
        std::vector<unsigned int> uints = {1,2,3};
        EXPECT_TRUE (    byteRange::isNonDecreasing(uints) );
    }
    {
        std::vector<unsigned int> uints = {10,20,30,45};
        EXPECT_TRUE (    byteRange::isNonDecreasing(uints) );
    }
    {
        std::list<unsigned int> uints = {10,20,45,30};
        EXPECT_FALSE(    byteRange::isNonDecreasing(uints) );
    }
    {
        std::list<unsigned int> uints = {10,20,30,35};
        EXPECT_TRUE (    byteRange::isNonDecreasing(uints) );
    }
}

TEST(byteRange, overlapsAnyIn){
    {
        byteRange c(20,10);
        {   //size 0 ranges can't overlap
            std::vector<byteRange> ranges = { byteRange(10,10), byteRange(30,10), byteRange(25,0)};
            EXPECT_FALSE(    c.overlapsAnyIn(ranges) );
        }
        {
            std::vector<byteRange> ranges = { byteRange(10,10), byteRange(30,10), byteRange(25,5)};
            EXPECT_TRUE (    c.overlapsAnyIn(ranges) );
        }
    }
    {
        byteRange range(20,10);
        unsigned int blockSize = 10;
        {
            std::vector<unsigned int> uints = {5,10,30,35};
            EXPECT_FALSE(    range.overlapsAnyIn(uints, blockSize) );
        }
        {
            std::vector<unsigned int> uints = {15};
            EXPECT_TRUE (    range.overlapsAnyIn(uints, blockSize) );
        }
    }
}

TEST(byteRange, findSizeOfLargestEmptySpace_and_fillEmptySpaces){
    {
        byteRange fillThisRange(0,50);
        std::list<byteRange> blocks = { byteRange(10,10), byteRange(25,10), byteRange(40,0)};
        EXPECT_EQ   (    15, byteRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<byteRange> copiesOfAddedBlocks;
        byteRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_FALSE(    byteRange::isNonDecreasingAndNonOverlapping(blocks)    );   //nondecreasing property broken by count 0 byteRange
        EXPECT_EQ   (    copiesOfAddedBlocks, (std::list<byteRange>{ byteRange(0,10), byteRange(20,5), byteRange(35,15) }) );
    }
    {
        byteRange fillThisRange(0,50);
        std::list<byteRange> blocks;
        EXPECT_EQ   (    50, byteRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<byteRange> copiesOfAddedBlocks;
        byteRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    byteRange::isNonDecreasingAndNonOverlapping(blocks)    );
        EXPECT_TRUE (    copiesOfAddedBlocks == std::list<byteRange>{ byteRange(0,50) } );
    }
    {
        byteRange fillThisRange(10,20);
        std::list<byteRange> blocks = { byteRange(0,15), byteRange(25,20)};
        EXPECT_EQ   (    10, byteRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<byteRange> copiesOfAddedBlocks;
        byteRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    byteRange::isNonDecreasingAndNonOverlapping(blocks)    );
        EXPECT_TRUE (    copiesOfAddedBlocks == std::list<byteRange>{ byteRange(15,10) } );
    }
    {
        byteRange fillThisRange(10,10);
        std::list<byteRange> blocks = { byteRange(0,15), byteRange(25,20)};
        EXPECT_EQ   (    5, byteRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<byteRange> copiesOfAddedBlocks;
        byteRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    byteRange::isNonDecreasingAndNonOverlapping(blocks)    );
        EXPECT_TRUE (    copiesOfAddedBlocks == std::list<byteRange>{ byteRange(15,5) } );
    }
    {
        byteRange fillThisRange(10,0);
        std::list<byteRange> blocks = { byteRange(0,15), byteRange(25,20)};
        EXPECT_EQ   (    0, byteRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<byteRange> copiesOfAddedBlocks;
        byteRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    byteRange::isNonDecreasingAndNonOverlapping(blocks)    );
        EXPECT_TRUE (    copiesOfAddedBlocks == std::list<byteRange>{} );
    }
    {
        byteRange fillThisRange(0,20);
        std::list<byteRange> blocks = { byteRange(0,10), byteRange(10,0), byteRange(10,10)};
        EXPECT_EQ   (    0, byteRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<byteRange> copiesOfAddedBlocks;
        byteRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    byteRange::isNonDecreasingAndNonOverlapping(blocks)    );
        EXPECT_TRUE (    copiesOfAddedBlocks.empty()                 );
    }
}

TEST(byteRange, isExactAscendingPartition){
    {   //size zero partitionRange
        byteRange partitionRange(10,0);
        std::list<byteRange> blocks = { byteRange(10,10), byteRange(20,5), byteRange(25,5) };
        EXPECT_FALSE(    byteRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
    {   //size zero blocks
        byteRange partitionRange(10,20);
        std::list<byteRange> blocks;
        EXPECT_FALSE(    byteRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
    {
        byteRange partitionRange(10,20);
        std::list<byteRange> blocks = { byteRange(10,10), byteRange(20,5), byteRange(25,5) };
        EXPECT_TRUE (    byteRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
    {   //blocks start before partitionRange
        byteRange partitionRange(10,20);
        std::list<byteRange> blocks = { byteRange(5,15), byteRange(20,5), byteRange(25,5) };
        EXPECT_FALSE(    byteRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
    {   //blocks extent past partitionRange
        byteRange partitionRange(10,20);
        std::list<byteRange> blocks = { byteRange(5,15), byteRange(20,5), byteRange(25,10) };
        EXPECT_FALSE(    byteRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
    {   //not ascending order
        byteRange partitionRange(10,20);
        std::list<byteRange> blocks = { byteRange(10,10), byteRange(25,5), byteRange(20,5) };
        EXPECT_FALSE(    byteRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
}
