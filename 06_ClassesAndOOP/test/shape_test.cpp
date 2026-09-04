#include "shape.h"

#include <gtest/gtest.h>

#include <sstream>

#include "circle.h"
#include "rectangle.h"
#include "square.h"

// operator<<は、実際の派生クラスの仮想関数(Name/Area/Perimeter)の結果を使って整形する
// (Shape&経由で呼び出しても多態性により正しい実装が呼ばれることを確認する)。
TEST(ShapeOperatorTest, StreamInsertionUsesVirtualFunctions) {
    const Rectangle rect(3.0, 4.0);
    const Shape& shape = rect;

    std::ostringstream oss;
    oss << shape;

    EXPECT_EQ(oss.str(), "Rectangle(面積=12, 周囲長=14)");
}

// operator==は面積の近似比較であり、形状が異なっても面積が等しければtrueになる仕様。
TEST(ShapeOperatorTest, EqualityComparesAreaAcrossDifferentShapeTypes) {
    const Square square(4.0);      // 面積16
    const Rectangle rect(2.0, 8.0);  // 面積16

    EXPECT_TRUE(square == rect);
}

TEST(ShapeOperatorTest, EqualityIsFalseWhenAreasDiffer) {
    const Square square(4.0);       // 面積16
    const Rectangle rect(2.0, 9.0);  // 面積18

    EXPECT_FALSE(square == rect);
}
