#include "number_guessing.h"

#include <gtest/gtest.h>

TEST(JudgeGuessTest, EqualToAnswerReturnsCorrect) {
    EXPECT_EQ(JudgeGuess(50, 50), GuessJudgment::Correct);
}

TEST(JudgeGuessTest, LessThanAnswerReturnsTooLow) {
    EXPECT_EQ(JudgeGuess(1, 50), GuessJudgment::TooLow);
    EXPECT_EQ(JudgeGuess(49, 50), GuessJudgment::TooLow);
}

TEST(JudgeGuessTest, GreaterThanAnswerReturnsTooHigh) {
    EXPECT_EQ(JudgeGuess(100, 50), GuessJudgment::TooHigh);
    EXPECT_EQ(JudgeGuess(51, 50), GuessJudgment::TooHigh);
}
