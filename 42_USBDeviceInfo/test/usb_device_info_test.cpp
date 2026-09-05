#include "usb_device_info.h"

#include <gtest/gtest.h>

using usb::ExtractSerialNumber;
using usb::ParseVidPid;
using usb::UsbVidPid;

TEST(ParseVidPidTest, ExtractsVidAndPidFromSimpleInstanceId) {
    const auto result = ParseVidPid(L"USB\\VID_046D&PID_C33C\\197633433932");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->vendorId, 0x046D);
    EXPECT_EQ(result->productId, 0xC33C);
}

// 複合デバイス(MI_xxを含む機能インターフェース)でもVID/PIDは同じ位置に現れる。
TEST(ParseVidPidTest, ExtractsVidAndPidFromCompositeDeviceInstanceId) {
    const auto result = ParseVidPid(L"USB\\VID_0B05&PID_19AF&MI_02\\6&DE2EAF5&0&0002");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->vendorId, 0x0B05);
    EXPECT_EQ(result->productId, 0x19AF);
}

// 実際のインスタンスIDは常に大文字だが、パース自体は大文字小文字を問わない。
TEST(ParseVidPidTest, IsCaseInsensitive) {
    const auto result = ParseVidPid(L"usb\\vid_046d&pid_c33c\\serial");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->vendorId, 0x046D);
    EXPECT_EQ(result->productId, 0xC33C);
}

TEST(ParseVidPidTest, ReturnsNulloptWhenVidOrPidMissing) {
    EXPECT_FALSE(ParseVidPid(L"USB\\ROOT_HUB30\\4&137CE2EE&0&0").has_value());
}

TEST(ParseVidPidTest, ReturnsNulloptForEmptyString) {
    EXPECT_FALSE(ParseVidPid(L"").has_value());
}

TEST(ExtractSerialNumberTest, ReturnsLastSegmentForSimpleDevice) {
    EXPECT_EQ(ExtractSerialNumber(L"USB\\VID_046D&PID_C33C\\197633433932"), L"197633433932");
}

// 複合デバイスの場合、最後のセグメントはシリアル番号ではなくロケーション文字列になる。
TEST(ExtractSerialNumberTest, ReturnsLocationStringForCompositeDevice) {
    EXPECT_EQ(ExtractSerialNumber(L"USB\\VID_0B05&PID_19AF&MI_02\\6&DE2EAF5&0&0002"),
              L"6&DE2EAF5&0&0002");
}

// '\'を含まない文字列に対しては、フォールバックとして全体をそのまま返す。
TEST(ExtractSerialNumberTest, ReturnsWholeStringWhenNoBackslash) {
    EXPECT_EQ(ExtractSerialNumber(L"NoBackslashHere"), L"NoBackslashHere");
}
