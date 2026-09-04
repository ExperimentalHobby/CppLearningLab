#include "reflect_format.h"

#include <gtest/gtest.h>

TEST(FormatReflectResultTest, ReturnsInputAsIs) {
    EXPECT_EQ(FormatReflectResult(L"hello"), L"hello");
}

// 空入力のときは専用のメッセージを表示する仕様。
TEST(FormatReflectResultTest, ReturnsPlaceholderForEmptyInput) {
    EXPECT_EQ(FormatReflectResult(L""), L"(空の入力が反映されました)");
}

TEST(FormatReflectCountTest, FormatsZero) {
    EXPECT_EQ(FormatReflectCount(0), L"反映回数: 0");
}

TEST(FormatReflectCountTest, FormatsPositiveCount) {
    EXPECT_EQ(FormatReflectCount(3), L"反映回数: 3");
}
