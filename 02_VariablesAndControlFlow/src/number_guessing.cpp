#include "number_guessing.h"

GuessJudgment JudgeGuess(int guess, int answer) {
    if (guess == answer) {
        return GuessJudgment::Correct;
    } else if (guess < answer) {
        return GuessJudgment::TooLow;
    } else {
        return GuessJudgment::TooHigh;
    }
}
