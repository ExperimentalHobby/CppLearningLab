#include "stats.h"

#include <gtest/gtest.h>

namespace {

std::vector<Student> SampleStudents() {
    return {
        {"Alice", 95}, {"Bob", 82}, {"Carol", 71}, {"Dave", 58}, {"Eve", 100},
    };
}

}  // namespace

TEST(SortByScoreDescendingTest, SortsFromHighestToLowest) {
    const auto sorted = SortByScoreDescending(SampleStudents());

    ASSERT_EQ(sorted.size(), 5u);
    EXPECT_EQ(sorted.front().name, "Eve");
    EXPECT_EQ(sorted.back().name, "Dave");
}

TEST(CountAtLeastTest, CountsStudentsAtOrAboveThreshold) {
    EXPECT_EQ(CountAtLeast(SampleStudents(), 80), 3u);
}

TEST(FilterAtLeastTest, KeepsOnlyStudentsAtOrAboveThreshold) {
    const auto filtered = FilterAtLeast(SampleStudents(), 90);

    ASSERT_EQ(filtered.size(), 2u);
    EXPECT_EQ(filtered[0].name, "Alice");
    EXPECT_EQ(filtered[1].name, "Eve");
}

TEST(SumScoresTest, SumsAllScores) {
    EXPECT_EQ(SumScores(SampleStudents()), 95 + 82 + 71 + 58 + 100);
}

TEST(AverageScoreTest, ComputesAverage) {
    EXPECT_DOUBLE_EQ(AverageScore(SampleStudents()), (95 + 82 + 71 + 58 + 100) / 5.0);
}

// studentsが空の場合は0除算を避けて0.0を返す仕様。
TEST(AverageScoreTest, ReturnsZeroForEmptyInput) {
    EXPECT_DOUBLE_EQ(AverageScore({}), 0.0);
}

TEST(GroupByGradeTest, GroupsStudentsByGrade) {
    const auto grouped = GroupByGrade(SampleStudents());

    ASSERT_TRUE(grouped.count("A"));
    EXPECT_EQ(grouped.at("A").size(), 2u);  // Alice(95), Eve(100)
    ASSERT_TRUE(grouped.count("F"));
    EXPECT_EQ(grouped.at("F").size(), 1u);  // Dave(58)
}

TEST(CountByGradeTest, CountsStudentsPerGrade) {
    const auto counts = CountByGrade(SampleStudents());

    EXPECT_EQ(counts.at("A"), 2);
    EXPECT_EQ(counts.at("F"), 1);
}

TEST(FindByNameTest, ReturnsIteratorToMatchingStudent) {
    const auto students = SampleStudents();

    const auto it = FindByName(students, "Carol");

    ASSERT_NE(it, students.end());
    EXPECT_EQ(it->score, 71);
}

// 見つからない場合はend()と等しいイテレータを返す(呼び出し側で比較して判定する)。
TEST(FindByNameTest, ReturnsEndIteratorWhenNotFound) {
    const auto students = SampleStudents();

    const auto it = FindByName(students, "Nobody");

    EXPECT_EQ(it, students.end());
}
