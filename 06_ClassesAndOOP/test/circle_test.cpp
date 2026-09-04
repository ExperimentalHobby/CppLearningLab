#include "circle.h"

#include <gtest/gtest.h>

TEST(CircleTest, ComputesAreaAndPerimeter) {
    const Circle circle(2.0);

    EXPECT_NEAR(circle.Area(), 12.566370614, 1e-6);
    EXPECT_NEAR(circle.Perimeter(), 12.566370614, 1e-6);
    EXPECT_EQ(circle.Name(), "Circle");
}

TEST(CircleTest, ThrowsWhenRadiusIsNotPositive) {
    EXPECT_THROW(Circle(0.0), std::invalid_argument);
    EXPECT_THROW(Circle(-1.0), std::invalid_argument);
}
