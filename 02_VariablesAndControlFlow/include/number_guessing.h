#ifndef NUMBER_GUESSING_H
#define NUMBER_GUESSING_H

// 予想(guess)と正解(answer)を比較した結果。
enum class GuessJudgment {
    Correct,  // 一致
    TooLow,   // guessがanswerより小さい
    TooHigh,  // guessがanswerより大きい
};

// guessとanswerを比較して判定結果を返す。標準入出力に依存しないため単体テストできる。
// 対話ループ本体(標準入力からの読み取り)はmain()側に残し、テスト対象外とする。
GuessJudgment JudgeGuess(int guess, int answer);

#endif  // NUMBER_GUESSING_H
