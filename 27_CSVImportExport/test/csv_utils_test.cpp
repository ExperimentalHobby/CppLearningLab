#include "csv_utils.h"

#include <gtest/gtest.h>

using csv::BuildCsvLine;
using csv::ParseCsv;
using csv::Row;

TEST(ParseCsvTest, ParsesSingleRow) {
    const auto rows = ParseCsv("a,b,c\n");

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0], (Row{"a", "b", "c"}));
}

TEST(ParseCsvTest, ParsesMultipleRows) {
    const auto rows = ParseCsv("a,b\nc,d\n");

    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0], (Row{"a", "b"}));
    EXPECT_EQ(rows[1], (Row{"c", "d"}));
}

// ファイル末尾に改行が無くても、最後の行を取りこぼさない。
TEST(ParseCsvTest, HandlesMissingTrailingNewline) {
    const auto rows = ParseCsv("a,b");

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0], (Row{"a", "b"}));
}

TEST(ParseCsvTest, ReturnsEmptyForEmptyInput) {
    EXPECT_TRUE(ParseCsv("").empty());
}

// \r\nの\rは無視され、\nだけが行区切りとして扱われる。
TEST(ParseCsvTest, TreatsCrLfAsSingleLineBreak) {
    const auto rows = ParseCsv("a,b\r\nc,d\r\n");

    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0], (Row{"a", "b"}));
}

TEST(ParseCsvTest, ParsesQuotedFieldContainingComma) {
    const auto rows = ParseCsv("\"a,b\",c\n");

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0], (Row{"a,b", "c"}));
}

// クォート内の改行は行区切りとして扱わず、1つの論理行のフィールドの一部になる。
TEST(ParseCsvTest, ParsesQuotedFieldContainingNewline) {
    const auto rows = ParseCsv("\"a\nb\",c\n");

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0], (Row{"a\nb", "c"}));
}

// クォート内の""はエスケープされた1個の"として扱われる。
TEST(ParseCsvTest, ParsesEscapedDoubleQuote) {
    const auto rows = ParseCsv("\"a\"\"b\",c\n");

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0], (Row{"a\"b", "c"}));
}

TEST(BuildCsvLineTest, JoinsFieldsWithComma) {
    EXPECT_EQ(BuildCsvLine({"a", "b", "c"}), "a,b,c");
}

// カンマを含むフィールドはダブルクォートで囲む。
TEST(BuildCsvLineTest, QuotesFieldContainingComma) {
    EXPECT_EQ(BuildCsvLine({"a,b", "c"}), "\"a,b\",c");
}

// ダブルクォートを含むフィールドは、囲んだ上で内部の"を""に二重化する。
TEST(BuildCsvLineTest, EscapesDoubleQuoteInsideField) {
    EXPECT_EQ(BuildCsvLine({"a\"b", "c"}), "\"a\"\"b\",c");
}

TEST(BuildCsvLineTest, QuotesFieldContainingNewline) {
    EXPECT_EQ(BuildCsvLine({"a\nb"}), "\"a\nb\"");
}

// BuildCsvLineで組み立てた行をParseCsvで読み戻すと、元のフィールドに一致する
// (エクスポート→インポートの往復整合性)。
TEST(CsvRoundTripTest, BuildThenParseRoundTripsSpecialCharacters) {
    const Row original = {"a,b", "c\"d", "e\nf", "plain"};
    const std::string line = BuildCsvLine(original);

    const auto rows = ParseCsv(line + "\n");

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0], original);
}
