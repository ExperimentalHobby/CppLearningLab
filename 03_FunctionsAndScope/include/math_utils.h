#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <string>

namespace math_utils {

// aとbの最大公約数を、ユークリッドの互除法（値渡し）で求める。
int Gcd(int a, int b);

// aとbの最小公倍数を求める（内部でGcdを利用する）。
int Lcm(int a, int b);

// nが素数かどうかを試し割り法で判定する。
bool IsPrime(int n);

// valueを[lo, hi]の範囲に収める（int版）。
int Clamp(int value, int lo, int hi);

// valueを[lo, hi]の範囲に収める（double版）。引数の型でint版/double版が
// 自動的に選ばれる（関数オーバーロード）。
double Clamp(double value, double lo, double hi);

// baseのexponent乗を返す。exponentを省略すると2乗（平方）を返す（デフォルト引数）。
long long Power(int base, int exponent = 2);

// aとbの値を参照渡しで交換する。呼び出し元の変数そのものが書き換わる。
void Swap(int& a, int& b);

// 数字文字列digitsの各桁の合計を返す（const参照渡し）。
// digitsに数字以外の文字が含まれる場合はstd::invalid_argumentを投げる。
int SumOfDigits(const std::string& digits);

}  // namespace math_utils

#endif  // MATH_UTILS_H
