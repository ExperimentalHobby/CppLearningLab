#include <iostream>
#include <stdexcept>

#include "math_utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    // コンソールの出力コードページをUTF-8に合わせる。既定のコードページ(Shift-JIS等)の
    // ままだと、UTF-8で出力した日本語が文字化けする。
    SetConsoleOutputCP(CP_UTF8);
#endif

    // 値渡し: Gcd/Lcmは引数のコピーを受け取って計算するので、呼び出し元の変数は変化しない。
    std::cout << "Gcd(24, 36) = " << math_utils::Gcd(24, 36) << std::endl;
    std::cout << "Lcm(4, 6) = " << math_utils::Lcm(4, 6) << std::endl;

    for (const int n : {2, 15, 17, 1, 97}) {
        std::cout << n << "は素数" << (math_utils::IsPrime(n) ? "" : "ではない") << std::endl;
    }

    // 関数オーバーロード: 引数の型(int/double)によってClampの呼び出し先が自動的に選ばれる。
    std::cout << "Clamp(15, 0, 10) = " << math_utils::Clamp(15, 0, 10) << std::endl;
    std::cout << "Clamp(3.5, 0.0, 3.0) = " << math_utils::Clamp(3.5, 0.0, 3.0) << std::endl;

    // デフォルト引数: exponentを省略すると2乗(平方)になる。
    std::cout << "Power(5) = " << math_utils::Power(5) << std::endl;
    std::cout << "Power(2, 10) = " << math_utils::Power(2, 10) << std::endl;

    // 参照渡し: Swapは呼び出し元の変数そのものを書き換える。
    int a = 1;
    int b = 2;
    std::cout << "Swap前: a=" << a << ", b=" << b << std::endl;
    math_utils::Swap(a, b);
    std::cout << "Swap後: a=" << a << ", b=" << b << std::endl;

    // const参照渡し: std::stringのような大きな型をコピーせずに読み取り専用で渡す。
    std::cout << "SumOfDigits(\"12345\") = " << math_utils::SumOfDigits("12345") << std::endl;
    try {
        // "12a45"は数字以外の文字を含むため、"= "を出力する前に例外が送出される。
        const int result = math_utils::SumOfDigits("12a45");
        std::cout << "SumOfDigits(\"12a45\") = " << result << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "SumOfDigits(\"12a45\") -> エラー: " << e.what() << std::endl;
    }

    return 0;
}
