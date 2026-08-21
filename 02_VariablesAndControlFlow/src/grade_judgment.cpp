#include <iostream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

// 0〜100点を10点刻みでA〜Fに判定する。switchのフォールスルーで
// 90点台と100点をまとめて"A"に判定している。
std::string JudgeGrade(int score) {
    switch (score / 10) {
        case 10:
        case 9:
            return "A";
        case 8:
            return "B";
        case 7:
            return "C";
        case 6:
            return "D";
        default:
            return "F";
    }
}

}  // namespace

int main() {
#ifdef _WIN32
    // コンソールの出力コードページをUTF-8に合わせる。既定のコードページ(Shift-JIS等)の
    // ままだと、UTF-8で出力した日本語（"点"など）が文字化けする。
    SetConsoleOutputCP(CP_UTF8);
#endif

    const std::vector<std::pair<std::string, int>> students = {
        {"Alice", 95}, {"Bob", 82}, {"Carol", 71}, {"Dave", 58}, {"Eve", 100},
    };

    // 構造化束縛(auto)+範囲forでname/scoreを分解しながら判定結果を出力する。
    for (const auto& [name, score] : students) {
        std::cout << name << ": " << score << "点 -> " << JudgeGrade(score) << std::endl;
    }

    return 0;
}
