#include "student.h"

#include <gtest/gtest.h>

TEST(DetermineGradeTest, ScoreInNineliesOrHundredReturnsA) {
    EXPECT_EQ(DetermineGrade(90), "A");
    EXPECT_EQ(DetermineGrade(100), "A");
}

TEST(DetermineGradeTest, ScoreBelowSixtyReturnsF) {
    EXPECT_EQ(DetermineGrade(59), "F");
}
