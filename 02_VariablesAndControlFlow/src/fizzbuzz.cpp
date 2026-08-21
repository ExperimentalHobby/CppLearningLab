#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    // コンソールの出力コードページをUTF-8に合わせる。本プログラムの出力はASCIIのみだが、
    // 同課題内の他プログラム(日本語出力あり)と同じ初期化を統一しておく。
    SetConsoleOutputCP(CP_UTF8);
#endif

    constexpr int kMin = 1;
    constexpr int kMax = 30;

    std::vector<int> numbers;
    for (int n = kMin; n <= kMax; ++n) {
        numbers.push_back(n);
    }

    // 範囲forとautoで、1〜30をFizzBuzzのルールに従って出力する。
    for (const auto& n : numbers) {
        if (n % 15 == 0) {
            std::cout << "FizzBuzz" << std::endl;
        } else if (n % 3 == 0) {
            std::cout << "Fizz" << std::endl;
        } else if (n % 5 == 0) {
            std::cout << "Buzz" << std::endl;
        } else {
            std::cout << n << std::endl;
        }
    }

    return 0;
}
