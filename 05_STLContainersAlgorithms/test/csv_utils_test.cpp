#include "csv_utils.h"

#include <gtest/gtest.h>

#include <sstream>

// 1行目はヘッダーとして読み飛ばす。
TEST(LoadStudentsFromCsvTest, SkipsHeaderRow) {
    std::istringstream input("name,score\nAlice,90\n");

    const auto students = LoadStudentsFromCsv(input);

    ASSERT_EQ(students.size(), 1u);
    EXPECT_EQ(students[0].name, "Alice");
    EXPECT_EQ(students[0].score, 90);
}

// 空行や、数値に変換できないscoreを持つ行は無視して読み飛ばす。
TEST(LoadStudentsFromCsvTest, IgnoresBlankAndInvalidRows) {
    std::istringstream input("name,score\nAlice,90\n\nBob,not-a-number\nCarol,80\n");

    const auto students = LoadStudentsFromCsv(input);

    ASSERT_EQ(students.size(), 2u);
    EXPECT_EQ(students[0].name, "Alice");
    EXPECT_EQ(students[1].name, "Carol");
}

TEST(LoadStudentsFromCsvTest, ReturnsEmptyVectorWhenOnlyHeaderExists) {
    std::istringstream input("name,score\n");

    const auto students = LoadStudentsFromCsv(input);

    EXPECT_TRUE(students.empty());
}
