#include "square.h"

#include <gtest/gtest.h>

// Area/PerimeterはRectangleの実装をそのまま継承して使う。
TEST(SquareTest, ComputesAreaAndPerimeterViaRectangle) {
    const Square square(5.0);

    EXPECT_DOUBLE_EQ(square.Area(), 25.0);
    EXPECT_DOUBLE_EQ(square.Perimeter(), 20.0);
}

// Name()だけはSquare独自にオーバーライドされている。
TEST(SquareTest, NameIsOverriddenToSquare) {
    const Square square(5.0);

    EXPECT_EQ(square.Name(), "Square");
}
