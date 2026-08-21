#include "math_utils.h"

#include <cctype>
#include <cmath>
#include <stdexcept>

namespace math_utils {

int Gcd(int a, int b) {
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        const int remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

int Lcm(int a, int b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return std::abs(a / Gcd(a, b) * b);
}

bool IsPrime(int n) {
    if (n < 2) {
        return false;
    }
    if (n % 2 == 0) {
        return n == 2;
    }
    for (int i = 3; static_cast<long long>(i) * i <= n; i += 2) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

int Clamp(int value, int lo, int hi) {
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

double Clamp(double value, double lo, double hi) {
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

long long Power(int base, int exponent) {
    // 学習用の簡易実装。exponentが負の場合は非対応。
    long long result = 1;
    const long long b = base;
    for (int i = 0; i < exponent; ++i) {
        result *= b;
    }
    return result;
}

void Swap(int& a, int& b) {
    const int temp = a;
    a = b;
    b = temp;
}

int SumOfDigits(const std::string& digits) {
    int sum = 0;
    for (const char c : digits) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            throw std::invalid_argument("digits must contain only digit characters: " + digits);
        }
        sum += c - '0';
    }
    return sum;
}

}  // namespace math_utils
