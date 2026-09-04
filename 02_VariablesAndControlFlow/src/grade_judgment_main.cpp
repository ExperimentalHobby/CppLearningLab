#include <iostream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "grade_judgment.h"

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
    // 判定ロジック自体はgrade_judgment.cppに切り出してあり、単体テストで検証済み。
    for (const auto& [name, score] : students) {
        std::cout << name << ": " << score << "点 -> " << JudgeGrade(score) << std::endl;
    }

    return 0;
}
