#include "display.h"

#include <gtest/gtest.h>

TEST(MaxTest, ReturnsLargerOfTwoInts) {
    EXPECT_EQ(Max(3, 7), 7);
    EXPECT_EQ(Max(7, 3), 7);
}

TEST(MaxTest, ReturnsLargerOfTwoDoubles) {
    EXPECT_DOUBLE_EQ(Max(1.5, 2.5), 2.5);
}

// プライマリテンプレート(汎用版): operator<<を経由して文字列化する。
TEST(ToDisplayStringTest, PrimaryTemplateStringifiesViaOstream) {
    EXPECT_EQ(ToDisplayString(42), "42");
    EXPECT_EQ(ToDisplayString(3.14), "3.14");
}

// bool版の明示的特殊化: "1"/"0"ではなく"true"/"false"を返す。
TEST(ToDisplayStringTest, BoolSpecializationReturnsTrueOrFalse) {
    EXPECT_EQ(ToDisplayString(true), "true");
    EXPECT_EQ(ToDisplayString(false), "false");
}

// std::string版の明示的特殊化: ダブルクォートで囲んで返す。
TEST(ToDisplayStringTest, StringSpecializationWrapsInQuotes) {
    EXPECT_EQ(ToDisplayString(std::string("hello")), "\"hello\"");
}
