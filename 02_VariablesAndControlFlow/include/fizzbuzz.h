#ifndef FIZZBUZZ_H
#define FIZZBUZZ_H

#include <string>

// nに対するFizzBuzzの判定結果を返す。
// 15の倍数は"FizzBuzz"、3の倍数は"Fizz"、5の倍数は"Buzz"、それ以外はnをそのまま
// 文字列化して返す。main()から切り出すことで、標準出力に依存せず単体テストできるようにする。
std::string FizzBuzzValue(int n);

#endif  // FIZZBUZZ_H
