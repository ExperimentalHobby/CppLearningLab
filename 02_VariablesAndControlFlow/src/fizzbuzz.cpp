#include "fizzbuzz.h"

// 15/3/5の倍数判定を上位の条件から順に評価する(15の倍数は3の倍数でも5の倍数でもあるため、
// 先に判定しないと"Fizz"や"Buzz"止まりになってしまう)。
std::string FizzBuzzValue(int n) {
    if (n % 15 == 0) {
        return "FizzBuzz";
    } else if (n % 3 == 0) {
        return "Fizz";
    } else if (n % 5 == 0) {
        return "Buzz";
    } else {
        return std::to_string(n);
    }
}
