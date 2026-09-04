#include "grade_judgment.h"

#include <gtest/gtest.h>

// 90〜100点は"A"(switchのフォールスルーでcase 10/9をまとめて判定している境界)。
TEST(JudgeGradeTest, ScoreInNineliesOrHundredReturnsA) {
    EXPECT_EQ(JudgeGrade(90), "A");
    EXPECT_EQ(JudgeGrade(99), "A");
    EXPECT_EQ(JudgeGrade(100), "A");
}

TEST(JudgeGradeTest, ScoreInEightiesReturnsB) {
    EXPECT_EQ(JudgeGrade(80), "B");
    EXPECT_EQ(JudgeGrade(89), "B");
}

TEST(JudgeGradeTest, ScoreInSeventiesReturnsC) {
    EXPECT_EQ(JudgeGrade(70), "C");
    EXPECT_EQ(JudgeGrade(79), "C");
}

TEST(JudgeGradeTest, ScoreInSixtiesReturnsD) {
    EXPECT_EQ(JudgeGrade(60), "D");
    EXPECT_EQ(JudgeGrade(69), "D");
}

// 60点未満はすべて"F"(switchのdefaultで判定)。
TEST(JudgeGradeTest, ScoreBelowSixtyReturnsF) {
    EXPECT_EQ(JudgeGrade(59), "F");
    EXPECT_EQ(JudgeGrade(0), "F");
}
