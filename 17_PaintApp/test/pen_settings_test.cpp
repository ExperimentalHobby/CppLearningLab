#include "pen_settings.h"

#include <gtest/gtest.h>

TEST(ColorToMenuIdTest, MapsKnownColors) {
    EXPECT_EQ(ColorToMenuId(RGB(0, 0, 0)), kIdColorBlack);
    EXPECT_EQ(ColorToMenuId(RGB(255, 0, 0)), kIdColorRed);
    EXPECT_EQ(ColorToMenuId(RGB(0, 0, 255)), kIdColorBlue);
    EXPECT_EQ(ColorToMenuId(RGB(0, 160, 0)), kIdColorGreen);
}

// メニューに無い色が渡された場合は黒として扱う。
TEST(ColorToMenuIdTest, FallsBackToBlackForUnknownColor) {
    EXPECT_EQ(ColorToMenuId(RGB(128, 128, 128)), kIdColorBlack);
}

TEST(WidthToMenuIdTest, MapsThinAtOrBelowOne) {
    EXPECT_EQ(WidthToMenuId(1), kIdWidthThin);
    EXPECT_EQ(WidthToMenuId(0), kIdWidthThin);
}

TEST(WidthToMenuIdTest, MapsThickAtOrAboveSix) {
    EXPECT_EQ(WidthToMenuId(6), kIdWidthThick);
    EXPECT_EQ(WidthToMenuId(10), kIdWidthThick);
}

TEST(WidthToMenuIdTest, MapsMediumBetweenThinAndThick) {
    EXPECT_EQ(WidthToMenuId(2), kIdWidthMedium);
    EXPECT_EQ(WidthToMenuId(5), kIdWidthMedium);
}
