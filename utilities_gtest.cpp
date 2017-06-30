#include "utilities.h"
#include "gtestDefs.h"
#include <gtest.h>

TEST(utilities, subtractClampToZero){
    EXPECT_EQ(0,            utilities::subtractClampToZero(UINT_MAX, UINT_MAX)  );
    EXPECT_EQ(UINT_MAX - 1, utilities::subtractClampToZero(UINT_MAX, 1)         );
    EXPECT_EQ(0,            utilities::subtractClampToZero(10, 10)              );
    EXPECT_EQ(0,            utilities::subtractClampToZero(10, 20)              );
    EXPECT_EQ(0,            utilities::subtractClampToZero(10, UINT_MAX)        );
    EXPECT_EQ(5,            utilities::subtractClampToZero(10, 5)               );
}

TEST(utilities, addClampToMax){
    EXPECT_EQ(UINT_MAX,     utilities::addClampToMax(0, UINT_MAX)               );
    EXPECT_EQ(UINT_MAX,     utilities::addClampToMax(1, UINT_MAX - 1)           );
    EXPECT_EQ(1,            utilities::addClampToMax(0, 1)                      );
    EXPECT_EQ(UINT_MAX,     utilities::addClampToMax(10, UINT_MAX - 10)         );
    EXPECT_EQ(UINT_MAX,     utilities::addClampToMax(20, UINT_MAX - 10)         );
    EXPECT_EQ(UINT_MAX,     utilities::addClampToMax(UINT_MAX - 10, 20)         );
    EXPECT_EQ(UINT_MAX,     utilities::addClampToMax(10, UINT_MAX)              );
    EXPECT_EQ(UINT_MAX,     utilities::addClampToMax(UINT_MAX, UINT_MAX)        );
    EXPECT_EQ(15,           utilities::addClampToMax(10, 5)                     );
}
