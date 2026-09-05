#include "hid_device.h"

#include <gtest/gtest.h>

using hid::HidError;
using hid::ReadReports;

// ReadReports()のcount<=0検証は、CreateFileWでのデバイスオープンより前に
// 行われるため、実際のHIDデバイスが無い環境でも(存在しないdevicePathでも)
// テストできる。ただし存在しないdevicePathを渡すとオープン失敗でも
// 同じHidError型が投げられるため、型だけでなくエラーメッセージに
// "count"を含むことまで確認し、意図した検証経路であることを区別する。
TEST(ReadReportsTest, ThrowsWhenCountIsZero) {
    try {
        ReadReports(L"dummy-device-path", 0);
        FAIL() << "HidErrorが投げられるはずでした";
    } catch (const HidError& e) {
        EXPECT_NE(std::string(e.what()).find("count"), std::string::npos);
    }
}

TEST(ReadReportsTest, ThrowsWhenCountIsNegative) {
    try {
        ReadReports(L"dummy-device-path", -1);
        FAIL() << "HidErrorが投げられるはずでした";
    } catch (const HidError& e) {
        EXPECT_NE(std::string(e.what()).find("count"), std::string::npos);
    }
}
