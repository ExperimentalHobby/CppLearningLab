#include "hid_report.h"

#include <gtest/gtest.h>

using hid::FormatBytes;
using hid::HidReport;
using hid::ParseReport;

TEST(ParseReportTest, SeparatesReportIdFromData) {
    const HidReport report = ParseReport({0x01, 0xAA, 0xBB, 0xCC});

    EXPECT_EQ(report.reportId, 0x01);
    EXPECT_EQ(report.data, (std::vector<uint8_t>{0xAA, 0xBB, 0xCC}));
}

// レポートIDのみでデータ部分が無い場合、dataは空になる。
TEST(ParseReportTest, HandlesReportIdOnlyWithNoData) {
    const HidReport report = ParseReport({0x05});

    EXPECT_EQ(report.reportId, 0x05);
    EXPECT_TRUE(report.data.empty());
}

TEST(ParseReportTest, ReturnsZeroReportIdForEmptyInput) {
    const HidReport report = ParseReport({});

    EXPECT_EQ(report.reportId, 0);
    EXPECT_TRUE(report.data.empty());
}

TEST(FormatBytesTest, FormatsAsSpaceSeparatedUppercaseHex) {
    EXPECT_EQ(FormatBytes({0x01, 0x02, 0x0A}), "01 02 0A");
}

TEST(FormatBytesTest, ReturnsEmptyStringForEmptyInput) {
    EXPECT_EQ(FormatBytes({}), "");
}

TEST(FormatBytesTest, PadsSingleDigitValuesWithLeadingZero) {
    EXPECT_EQ(FormatBytes({0x00, 0xFF}), "00 FF");
}
