#include "fizzbuzz.h"

#include <gtest/gtest.h>

// 3の倍数(15の倍数を除く)は"Fizz"を返す。
TEST(FizzBuzzValueTest, MultipleOfThreeReturnsFizz) {
    EXPECT_EQ(FizzBuzzValue(3), "Fizz");
    EXPECT_EQ(FizzBuzzValue(9), "Fizz");
}

// 5の倍数(15の倍数を除く)は"Buzz"を返す。
TEST(FizzBuzzValueTest, MultipleOfFiveReturnsBuzz) {
    EXPECT_EQ(FizzBuzzValue(5), "Buzz");
    EXPECT_EQ(FizzBuzzValue(20), "Buzz");
}

// 15の倍数は"FizzBuzz"を返す(3の倍数判定・5の倍数判定より優先される)。
TEST(FizzBuzzValueTest, MultipleOfFifteenReturnsFizzBuzz) {
    EXPECT_EQ(FizzBuzzValue(15), "FizzBuzz");
    EXPECT_EQ(FizzBuzzValue(30), "FizzBuzz");
}

// 3の倍数でも5の倍数でもない数は、数値をそのまま文字列化して返す。
TEST(FizzBuzzValueTest, OtherwiseReturnsNumberAsString) {
    EXPECT_EQ(FizzBuzzValue(1), "1");
    EXPECT_EQ(FizzBuzzValue(7), "7");
}
