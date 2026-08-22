#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <string>

// GUI(Win32のボタン群)から独立した、電卓の状態管理とロジックだけを持つクラス。
// 「UIとロジックの分離」の実践: このクラスはWin32を一切知らないので、
// 単体でインスタンス化してテストできる。
class Calculator {
public:
    Calculator() = default;

    void InputDigit(char digit);
    void InputDecimalPoint();
    void InputOperator(char op);  // '+', '-', '*', '/'
    void Equals();
    void Clear();

    // 現在の表示内容(入力中の数値、直前の計算結果、またはエラー)を文字列で返す。
    std::string Display() const;
    bool HasError() const { return hasError_; }

private:
    void CommitPendingOperation();

    double accumulator_ = 0.0;      // 直前までの計算結果
    char pendingOperator_ = '\0';   // 保留中の演算子('\0'ならなし)
    std::string currentInput_ = "0";  // 入力中の数値の文字列表現
    bool startingNewInput_ = true;  // 次の数字入力で新しい数値として開始するか
    bool hasError_ = false;         // ゼロ除算等でエラー状態になったか
};

#endif  // CALCULATOR_H
