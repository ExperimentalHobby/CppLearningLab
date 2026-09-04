#include "math_utils.h"

#include <gtest/gtest.h>

using namespace math_utils;

TEST(GcdTest, ComputesGreatestCommonDivisor) {
    EXPECT_EQ(Gcd(12, 18), 6);
    EXPECT_EQ(Gcd(7, 13), 1);
}

// 負の値が渡されても、絶対値を取ってから計算するため正の値を返す。
TEST(GcdTest, HandlesNegativeValues) {
    EXPECT_EQ(Gcd(-12, 18), 6);
}

TEST(LcmTest, ComputesLeastCommonMultiple) {
    EXPECT_EQ(Lcm(4, 6), 12);
}

// 片方が0の場合はGcdが0除算を起こさないよう、先に0を返す仕様。
TEST(LcmTest, ReturnsZeroWhenEitherArgumentIsZero) {
    EXPECT_EQ(Lcm(0, 5), 0);
    EXPECT_EQ(Lcm(5, 0), 0);
}

TEST(IsPrimeTest, RecognizesPrimeNumbers) {
    EXPECT_TRUE(IsPrime(2));
    EXPECT_TRUE(IsPrime(3));
    EXPECT_TRUE(IsPrime(97));
}

TEST(IsPrimeTest, RecognizesNonPrimeNumbers) {
    EXPECT_FALSE(IsPrime(1));
    EXPECT_FALSE(IsPrime(0));
    EXPECT_FALSE(IsPrime(-5));
    EXPECT_FALSE(IsPrime(4));
    EXPECT_FALSE(IsPrime(100));
}

TEST(ClampTest, ClampsIntWithinRange) {
    EXPECT_EQ(Clamp(5, 0, 10), 5);
    EXPECT_EQ(Clamp(-1, 0, 10), 0);
    EXPECT_EQ(Clamp(11, 0, 10), 10);
}

TEST(ClampTest, ClampsDoubleWithinRange) {
    EXPECT_DOUBLE_EQ(Clamp(5.5, 0.0, 10.0), 5.5);
    EXPECT_DOUBLE_EQ(Clamp(-1.0, 0.0, 10.0), 0.0);
    EXPECT_DOUBLE_EQ(Clamp(11.0, 0.0, 10.0), 10.0);
}

TEST(PowerTest, ComputesBaseToExponent) {
    EXPECT_EQ(Power(2, 10), 1024);
    EXPECT_EQ(Power(3, 0), 1);
}

// exponentを省略すると2乗(デフォルト引数)。
TEST(PowerTest, DefaultsToSquare) {
    EXPECT_EQ(Power(5), 25);
}

TEST(SwapTest, SwapsTwoValues) {
    int a = 1;
    int b = 2;
    Swap(a, b);
    EXPECT_EQ(a, 2);
    EXPECT_EQ(b, 1);
}

TEST(SumOfDigitsTest, SumsDigitCharacters) {
    EXPECT_EQ(SumOfDigits("12345"), 15);
    EXPECT_EQ(SumOfDigits("0"), 0);
}

TEST(SumOfDigitsTest, ThrowsOnNonDigitCharacter) {
    EXPECT_THROW(SumOfDigits("12a45"), std::invalid_argument);
}
