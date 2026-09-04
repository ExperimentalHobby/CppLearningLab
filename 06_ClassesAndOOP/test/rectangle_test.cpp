#include "rectangle.h"

#include <gtest/gtest.h>

TEST(RectangleTest, ComputesAreaAndPerimeter) {
    const Rectangle rect(3.0, 4.0);

    EXPECT_DOUBLE_EQ(rect.Area(), 12.0);
    EXPECT_DOUBLE_EQ(rect.Perimeter(), 14.0);
    EXPECT_EQ(rect.Name(), "Rectangle");
}

TEST(RectangleTest, ThrowsWhenWidthOrHeightIsNotPositive) {
    EXPECT_THROW(Rectangle(0.0, 4.0), std::invalid_argument);
    EXPECT_THROW(Rectangle(3.0, -1.0), std::invalid_argument);
}
