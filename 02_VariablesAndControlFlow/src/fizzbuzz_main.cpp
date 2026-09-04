#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "fizzbuzz.h"

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

    // 範囲forとautoで、1〜30の判定結果を出力する。判定ロジック自体はfizzbuzz.cppに
    // 切り出してあり、単体テストで検証済み。
    for (const auto& n : numbers) {
        std::cout << FizzBuzzValue(n) << std::endl;
    }

    return 0;
}
