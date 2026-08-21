#include <iostream>
#include <random>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // コンソールの入出力コードページをUTF-8に合わせる。既定のコードページ(Shift-JIS等)の
    // ままだと、UTF-8で出力した日本語のメッセージが文字化けする。
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    constexpr int kMin = 1;
    constexpr int kMax = 100;
    constexpr int kMaxAttempts = 10;

    // 通常は乱数デバイスでシードするが、動作確認をしやすいように
    // コマンドライン引数でシードを上書きできるようにしている。
    unsigned int seed = std::random_device{}();
    if (argc > 1) {
        seed = static_cast<unsigned int>(std::stoul(argv[1]));
    }

    auto engine = std::mt19937(seed);
    auto dist = std::uniform_int_distribution<int>(kMin, kMax);
    const int answer = dist(engine);

    std::cout << kMin << "から" << kMax << "までの数を当ててください。("
              << kMaxAttempts << "回以内)" << std::endl;

    for (int attempt = 1; attempt <= kMaxAttempts; ++attempt) {
        std::cout << "第" << attempt << "回目の予想: ";
        int guess = 0;
        if (!(std::cin >> guess)) {
            std::cout << "入力が不正です。終了します。" << std::endl;
            return 1;
        }

        if (guess == answer) {
            std::cout << "正解！ " << attempt << "回で当てました。" << std::endl;
            return 0;
        } else if (guess < answer) {
            std::cout << "もっと大きい数です。" << std::endl;
        } else {
            std::cout << "もっと小さい数です。" << std::endl;
        }
    }

    std::cout << "残念、" << kMaxAttempts << "回以内に当てられませんでした。正解は " << answer
              << " でした。" << std::endl;
    return 0;
}
