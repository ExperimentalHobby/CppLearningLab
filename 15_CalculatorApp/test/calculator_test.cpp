#include "calculator.h"

#include <gtest/gtest.h>

TEST(CalculatorTest, InitialDisplayIsZero) {
    Calculator calc;
    EXPECT_EQ(calc.Display(), "0");
}

TEST(CalculatorTest, InputDigitAppendsToDisplay) {
    Calculator calc;
    calc.InputDigit('1');
    calc.InputDigit('2');
    calc.InputDigit('3');
    EXPECT_EQ(calc.Display(), "123");
}

// 表示が"0"のときに数字を入力すると、先頭のゼロは置き換わる("05"にはならない)。
TEST(CalculatorTest, InputDigitReplacesLeadingZero) {
    Calculator calc;
    calc.InputDigit('5');
    EXPECT_EQ(calc.Display(), "5");
}

TEST(CalculatorTest, InputDecimalPointAllowsFractionalInput) {
    Calculator calc;
    calc.InputDigit('1');
    calc.InputDecimalPoint();
    calc.InputDigit('5');
    EXPECT_EQ(calc.Display(), "1.5");
}

// 小数点は1つの数値につき1個まで。2回目の入力は無視される。
TEST(CalculatorTest, InputDecimalPointIsIgnoredWhenAlreadyPresent) {
    Calculator calc;
    calc.InputDigit('1');
    calc.InputDecimalPoint();
    calc.InputDecimalPoint();
    calc.InputDigit('5');
    EXPECT_EQ(calc.Display(), "1.5");
}

TEST(CalculatorTest, AddsTwoNumbers) {
    Calculator calc;
    calc.InputDigit('1');
    calc.InputOperator('+');
    calc.InputDigit('2');
    calc.Equals();
    EXPECT_EQ(calc.Display(), "3");
}

TEST(CalculatorTest, SubtractsTwoNumbers) {
    Calculator calc;
    calc.InputDigit('5');
    calc.InputOperator('-');
    calc.InputDigit('2');
    calc.Equals();
    EXPECT_EQ(calc.Display(), "3");
}

TEST(CalculatorTest, MultipliesTwoNumbers) {
    Calculator calc;
    calc.InputDigit('4');
    calc.InputOperator('*');
    calc.InputDigit('3');
    calc.Equals();
    EXPECT_EQ(calc.Display(), "12");
}

TEST(CalculatorTest, DividesTwoNumbers) {
    Calculator calc;
    calc.InputDigit('6');
    calc.InputOperator('/');
    calc.InputDigit('3');
    calc.Equals();
    EXPECT_EQ(calc.Display(), "2");
}

// 演算子を続けて入力すると、直前の演算が確定されてから次の演算子が保留される
// (電卓アプリでよくある「連続演算」の挙動)。
TEST(CalculatorTest, ChainsOperatorsWithoutPressingEquals) {
    Calculator calc;
    calc.InputDigit('1');
    calc.InputOperator('+');
    calc.InputDigit('2');
    calc.InputOperator('+');
    calc.InputDigit('3');
    calc.Equals();
    EXPECT_EQ(calc.Display(), "6");
}

TEST(CalculatorTest, DivisionByZeroSetsErrorState) {
    Calculator calc;
    calc.InputDigit('1');
    calc.InputOperator('/');
    calc.InputDigit('0');
    calc.Equals();

    EXPECT_TRUE(calc.HasError());
    EXPECT_EQ(calc.Display(), "Error");
}

// エラー状態になった後は、Clear()するまで入力操作を受け付けない。
TEST(CalculatorTest, IgnoresInputWhileInErrorState) {
    Calculator calc;
    calc.InputDigit('1');
    calc.InputOperator('/');
    calc.InputDigit('0');
    calc.Equals();

    calc.InputDigit('9');
    EXPECT_EQ(calc.Display(), "Error");
}

TEST(CalculatorTest, ClearRecoversFromErrorState) {
    Calculator calc;
    calc.InputDigit('1');
    calc.InputOperator('/');
    calc.InputDigit('0');
    calc.Equals();

    calc.Clear();

    EXPECT_FALSE(calc.HasError());
    EXPECT_EQ(calc.Display(), "0");
}
