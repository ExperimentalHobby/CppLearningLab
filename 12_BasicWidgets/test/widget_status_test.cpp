#include "widget_status.h"

#include <gtest/gtest.h>

TEST(FormatApplyStatusTest, IncludesNameAndOnWhenNotifyEnabled) {
    EXPECT_EQ(FormatApplyStatus(L"Alice", true), L"適用: 名前=\"Alice\", 通知=ON");
}

TEST(FormatApplyStatusTest, IncludesOffWhenNotifyDisabled) {
    EXPECT_EQ(FormatApplyStatus(L"Alice", false), L"適用: 名前=\"Alice\", 通知=OFF");
}

// 名前欄が空でも(バリデーションせず)そのまま反映する仕様。
TEST(FormatApplyStatusTest, HandlesEmptyName) {
    EXPECT_EQ(FormatApplyStatus(L"", true), L"適用: 名前=\"\", 通知=ON");
}
