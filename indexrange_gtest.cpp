#include "indexrange.h"
#include "gtestDefs.h"
#include <gtest.h>

//based on byteRange_gtest.cpp

TEST(indexRange, constructors){
    {
        indexRange a;
        EXPECT_TRUE( 0 == a.start && 0 == a.end );
    }
    {
        indexRange a(10,20);
        EXPECT_TRUE( 10 == a.start && 20 == a.end );
    }
}

TEST(indexRange, move){
    {
        indexRange a(0,10);
        a.move(0);
        EXPECT_EQ(indexRange(0,10), a);
    }
    {
        indexRange a(0,10);
        a.move(20);
        EXPECT_EQ(indexRange(20,30), a);
    }
}

TEST(indexRange, comparison_operators){
    EXPECT_TRUE (indexRange( 0,10) == indexRange( 0,10));
    EXPECT_TRUE (indexRange( 0, 0) == indexRange(10, 0));
    EXPECT_FALSE(indexRange( 0, 0) == indexRange( 0,10));

    EXPECT_FALSE(indexRange( 0,10) != indexRange( 0,10));
    EXPECT_FALSE(indexRange( 0, 0) != indexRange(10, 0));
    EXPECT_TRUE (indexRange( 0, 0) != indexRange( 0,10));

    // < compares start index only
    EXPECT_FALSE(indexRange( 0, 0) < indexRange( 0, 0));
    EXPECT_FALSE(indexRange( 0, 0) < indexRange( 0,10));
    EXPECT_TRUE (indexRange( 0, 0) < indexRange(10, 0));
    EXPECT_TRUE (indexRange( 0, 0) < indexRange(10,10));
    EXPECT_TRUE (indexRange(10,20) < indexRange(20, 0));
    EXPECT_TRUE (indexRange(10,20) < indexRange(20,20));
    EXPECT_TRUE (indexRange(10,20) < indexRange(20,40));
}

TEST(indexRange, count){
    //size zero
    EXPECT_EQ(0,    indexRange( 0, 0).count());
    EXPECT_EQ(0,    indexRange(10,10).count());
    EXPECT_EQ(0,    indexRange(10, 0).count());
    EXPECT_EQ(0,    indexRange(UINT_MAX, UINT_MAX).count());

    EXPECT_EQ(10,   indexRange( 0,10).count());
    EXPECT_EQ(10,   indexRange(10,20).count());
    EXPECT_EQ(10,   indexRange(UINT_MAX-10, UINT_MAX).count());
}

TEST(indexRange, contains){
    //size zero
    indexRange a(10,10);
    EXPECT_FALSE(   a.contains(0)                   );
    EXPECT_FALSE(   a.contains(9)                   );
    EXPECT_FALSE(   a.contains(10)                  );
    EXPECT_FALSE(   a.contains(11)                  );

    indexRange b(20,30);

    EXPECT_FALSE(   b.contains(0)                   );
    EXPECT_TRUE (   b.contains(20)                  );
    EXPECT_TRUE (   b.contains(29)                  );
    EXPECT_FALSE(   b.contains(30)                  );
}

TEST(indexRange, overlaps){
    indexRange a(20,30);

    //identity
    EXPECT_TRUE (   a.overlaps(a)                   );

    //smaller
    EXPECT_FALSE(   a.overlaps(indexRange(15,20))   );
    EXPECT_TRUE (   a.overlaps(indexRange(20,25))   );
    EXPECT_TRUE (   a.overlaps(indexRange(25,30))   );
    EXPECT_FALSE(   a.overlaps(indexRange(30,35))   );

    //same size
    EXPECT_FALSE(   a.overlaps(indexRange(10,20))   );
    EXPECT_TRUE (   a.overlaps(indexRange(15,25))   );
    EXPECT_TRUE (   a.overlaps(indexRange(20,30))   );
    EXPECT_TRUE (   a.overlaps(indexRange(25,35))   );
    EXPECT_FALSE(   a.overlaps(indexRange(30,40))   );

    //larger
    EXPECT_FALSE(   a.overlaps(indexRange( 0,20))   );
    EXPECT_TRUE (   a.overlaps(indexRange( 5,25))   );
    EXPECT_TRUE (   a.overlaps(indexRange(10,30))   );
    EXPECT_TRUE (   a.overlaps(indexRange(15,35))   );
    EXPECT_TRUE (   a.overlaps(indexRange(20,40))   );
    EXPECT_TRUE (   a.overlaps(indexRange(25,45))   );
    EXPECT_FALSE(   a.overlaps(indexRange(30,50))   );

    //zero size
    EXPECT_FALSE(   a.overlaps(indexRange(15,15))   );
    EXPECT_FALSE(   a.overlaps(indexRange(20,20))   );
    EXPECT_FALSE(   a.overlaps(indexRange(25,25))   );
    EXPECT_FALSE(   a.overlaps(indexRange(30,30))   );

    indexRange b(20,20);

    EXPECT_FALSE(   b.overlaps(b)                   );
    EXPECT_FALSE(   b.overlaps(indexRange(10,20))   );
    EXPECT_FALSE(   b.overlaps(indexRange(15,25))   );
    EXPECT_FALSE(   b.overlaps(indexRange(20,30))   );
}

TEST(indexRange, getIntersection){
    {
        //same count
        indexRange a(20,30);
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange( 0,10))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange(10,20))  );
        EXPECT_EQ(  indexRange(20,25), a.getIntersection(indexRange(15,25))  );
        EXPECT_EQ(  indexRange(20,30), a.getIntersection(indexRange(20,30))  );
        EXPECT_EQ(  indexRange(25,30), a.getIntersection(indexRange(25,35))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange(30,40))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange(40,50))  );
    }
    {
        //larger
        indexRange a(30,40);
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange( 0,20))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange(10,30))  );
        EXPECT_EQ(  indexRange(30,35), a.getIntersection(indexRange(15,35))  );
        EXPECT_EQ(  indexRange(30,40), a.getIntersection(indexRange(20,40))  );
        EXPECT_EQ(  indexRange(30,40), a.getIntersection(indexRange(25,45))  );
        EXPECT_EQ(  indexRange(30,40), a.getIntersection(indexRange(30,50))  );
        EXPECT_EQ(  indexRange(35,40), a.getIntersection(indexRange(35,55))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange(40,60))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange(50,70))  );
    }
    {
        //smaller
        indexRange a(10,20);
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange( 0, 5))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange( 5,10))  );
        EXPECT_EQ(  indexRange(10,15), a.getIntersection(indexRange(10,15))  );
        EXPECT_EQ(  indexRange(15,20), a.getIntersection(indexRange(15,20))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange(20,25))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange(25,30))  );
    }
    {
        //zero count
        indexRange a(10,10);
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange( 0, 5))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange( 0,20))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange( 5,10))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange( 5,15))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange(10,15))  );
        EXPECT_EQ(  indexRange( 0, 0), a.getIntersection(indexRange(15,20))  );
    }
}

TEST(indexRange, overlapsAnyIn){
    {
        indexRange c(20,30);
        {   //size 0 ranges can't overlap
            std::vector<indexRange> ranges = { indexRange(10,20), indexRange(30,40), indexRange(25,25)};
            EXPECT_FALSE(    c.overlapsAnyIn(ranges) );
        }
        {
            std::vector<indexRange> ranges = { indexRange(10,20), indexRange(30,40), indexRange(25,30)};
            EXPECT_TRUE (    c.overlapsAnyIn(ranges) );
        }
    }
    {
        indexRange range(20,30);
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

TEST(indexRange, isNonDecreasingAndNonOverlapping){
    {
        std::vector<indexRange> ranges = { indexRange(10,20), indexRange(20,30), indexRange(35,35), indexRange(35,45) };
        EXPECT_TRUE (    indexRange::isNonDecreasingAndNonOverlapping(ranges)   );
    }
    {   //decreasing
        std::vector<indexRange> ranges = { indexRange(10,20), indexRange(30,35), indexRange(20,30), indexRange(35,45) };
        EXPECT_FALSE(    indexRange::isNonDecreasingAndNonOverlapping(ranges)   );

        ranges[1].start = 35; //set to (35,35)
        //ranges with zero count are ignored, this will succeed
        EXPECT_TRUE (    indexRange::isNonDecreasingAndNonOverlapping(ranges)   );
    }
    {
        std::vector<unsigned int> uints = {1,2,3};
        unsigned int blockSize = 0;
        EXPECT_TRUE (    indexRange::isNonDecreasingAndNonOverlapping(uints, blockSize) );
    }
    {
        std::vector<unsigned int> uints = {10,20,30,45};
        unsigned int blockSize = 10;
        EXPECT_TRUE (    indexRange::isNonDecreasingAndNonOverlapping(uints, blockSize) );
    }
    {   //decreasing
        std::list<unsigned int> uints = {10,20,45,30};
        unsigned int blockSize = 10;
        EXPECT_FALSE(    indexRange::isNonDecreasingAndNonOverlapping(uints, blockSize) );
    }
    {   //overlapping
        std::list<unsigned int> uints = {10,20,30,35};
        unsigned int blockSize = 10;
        EXPECT_FALSE(    indexRange::isNonDecreasingAndNonOverlapping(uints, blockSize) );
    }
}

TEST(indexRange, isNonDecreasing){
    {
        std::vector<unsigned int> uints = {1,2,3};
        EXPECT_TRUE (    indexRange::isNonDecreasing(uints) );
    }
    {
        std::vector<unsigned int> uints = {10,20,30,45};
        EXPECT_TRUE (    indexRange::isNonDecreasing(uints) );
    }
    {
        std::list<unsigned int> uints = {10,20,45,30};
        EXPECT_FALSE(    indexRange::isNonDecreasing(uints) );
    }
    {
        std::list<unsigned int> uints = {10,20,30,35};
        EXPECT_TRUE (    indexRange::isNonDecreasing(uints) );
    }
}

TEST(indexRange, isNonOverlapping){
    {
        std::vector<unsigned int> uints = {1,2,3};
        unsigned int blockSize = 0;
        EXPECT_TRUE (    indexRange::isNonOverlapping(uints, blockSize) );
    }
    {
        std::vector<unsigned int> uints = {10,20,30,45};
        unsigned int blockSize = 10;
        EXPECT_TRUE (    indexRange::isNonOverlapping(uints, blockSize) );
    }
    {
        std::list<unsigned int> uints = {10,20,45,30};
        unsigned int blockSize = 10;
        EXPECT_TRUE (    indexRange::isNonOverlapping(uints, blockSize) );
    }
    {
        std::list<unsigned int> uints = {10,20,30,35};
        unsigned int blockSize = 10;
        EXPECT_FALSE(    indexRange::isNonOverlapping(uints, blockSize) );
    }
    {
        std::vector<indexRange> ranges = { indexRange(40,50), indexRange(20,30), indexRange(30,30), indexRange(10,20) };
        EXPECT_TRUE (    indexRange::isNonOverlapping(ranges)   );
    }
    {
        std::vector<indexRange> ranges = { indexRange(40,50), indexRange(20,35), indexRange(30,30), indexRange(10,20) };
        EXPECT_TRUE (    indexRange::isNonOverlapping(ranges)   );
    }
    {
        std::vector<indexRange> ranges = { indexRange(40,50), indexRange(20,35), indexRange(30,31), indexRange(10,20) };
        EXPECT_FALSE(    indexRange::isNonOverlapping(ranges)   );
    }
}

TEST(indexRange, findSizeOfLargestEmptySpace_and_fillEmptySpaces){
    {
        indexRange fillThisRange(0,50);
        std::list<indexRange> blocks = { indexRange(10,20), indexRange(25,35), indexRange(40,40)};
        EXPECT_EQ   (    15, indexRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<indexRange> copiesOfAddedBlocks;
        indexRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    indexRange::isNonDecreasingAndNonOverlapping(blocks)    );   //nondecreasing property NOT broken by count 0 indexRange
        EXPECT_EQ   (    copiesOfAddedBlocks, (std::list<indexRange>{ indexRange(0,10), indexRange(20,25), indexRange(35,50) }) );
    }
    {
        indexRange fillThisRange(0,50);
        std::list<indexRange> blocks;
        EXPECT_EQ   (    50, indexRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<indexRange> copiesOfAddedBlocks;
        indexRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    indexRange::isNonDecreasingAndNonOverlapping(blocks)    );
        EXPECT_TRUE (    copiesOfAddedBlocks == std::list<indexRange>{ indexRange(0,50) } );
    }
    {
        indexRange fillThisRange(10,30);
        std::list<indexRange> blocks = { indexRange(0,15), indexRange(25,45)};
        EXPECT_EQ   (    10, indexRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<indexRange> copiesOfAddedBlocks;
        indexRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    indexRange::isNonDecreasingAndNonOverlapping(blocks)    );
        EXPECT_TRUE (    copiesOfAddedBlocks == std::list<indexRange>{ indexRange(15,25) } );
    }
    {
        indexRange fillThisRange(10,20);
        std::list<indexRange> blocks = { indexRange(0,15), indexRange(25,45)};
        EXPECT_EQ   (    5, indexRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<indexRange> copiesOfAddedBlocks;
        indexRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    indexRange::isNonDecreasingAndNonOverlapping(blocks)    );
        EXPECT_TRUE (    copiesOfAddedBlocks == std::list<indexRange>{ indexRange(15,20) } );
    }
    {
        indexRange fillThisRange(10,10);
        std::list<indexRange> blocks = { indexRange(0,15), indexRange(25,45)};
        EXPECT_EQ   (    0, indexRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<indexRange> copiesOfAddedBlocks;
        indexRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    indexRange::isNonDecreasingAndNonOverlapping(blocks)    );
        EXPECT_TRUE (    copiesOfAddedBlocks == std::list<indexRange>{} );
    }
    {
        indexRange fillThisRange(0,20);
        std::list<indexRange> blocks = { indexRange(0,10), indexRange(10,10), indexRange(10,20)};
        EXPECT_EQ   (    0, indexRange::findSizeOfLargestEmptySpace(fillThisRange, blocks)  );

        std::list<indexRange> copiesOfAddedBlocks;
        indexRange::fillEmptySpaces(fillThisRange, blocks, copiesOfAddedBlocks);
        EXPECT_TRUE (    indexRange::isNonDecreasingAndNonOverlapping(blocks)    );
        EXPECT_TRUE (    copiesOfAddedBlocks.empty()                 );
    }
}

TEST(indexRange, isExactAscendingPartition){
    {   //size zero partitionRange
        indexRange partitionRange(10,10);
        std::list<indexRange> blocks = { indexRange(10,20), indexRange(20,25), indexRange(25,30) };
        EXPECT_FALSE(    indexRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
    {   //size zero blocks
        indexRange partitionRange(10,30);
        std::list<indexRange> blocks;
        EXPECT_FALSE(    indexRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
    {
        indexRange partitionRange(10,30);
        std::list<indexRange> blocks = { indexRange(10,20), indexRange(20,25), indexRange(25,30) };
        EXPECT_TRUE (    indexRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
    {   //blocks start before partitionRange
        indexRange partitionRange(10,30);
        std::list<indexRange> blocks = { indexRange(5,20), indexRange(20,25), indexRange(25,30) };
        EXPECT_FALSE(    indexRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
    {   //blocks extent past partitionRange
        indexRange partitionRange(10,30);
        std::list<indexRange> blocks = { indexRange(5,20), indexRange(20,25), indexRange(25,35) };
        EXPECT_FALSE(    indexRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
    {   //not ascending order
        indexRange partitionRange(10,30);
        std::list<indexRange> blocks = { indexRange(10,20), indexRange(25,30), indexRange(20,25) };
        EXPECT_FALSE(    indexRange::isExactAscendingPartition(partitionRange, blocks)   );
    }
}
