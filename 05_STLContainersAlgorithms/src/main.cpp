#include <fstream>
#include <iostream>
#include <string>

#include "csv_utils.h"
#include "stats.h"
#include "student.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

void PrintStudent(const Student& s) {
    std::cout << "  " << s.name << ": " << s.score << "点 (" << DetermineGrade(s.score) << ")"
              << std::endl;
}

}  // namespace

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    const std::string path = (argc > 1) ? argv[1] : std::string(DATA_DIR) + "/students.csv";

    std::ifstream file(path);
    if (!file) {
        std::cout << "CSVファイルを開けませんでした: " << path << std::endl;
        return 1;
    }

    const std::vector<Student> students = LoadStudentsFromCsv(file);
    std::cout << "読み込んだ学生数: " << students.size() << std::endl;

    std::cout << "\n=== 点数降順ソート(std::sort + ラムダ) ===" << std::endl;
    for (const auto& s : SortByScoreDescending(students)) {
        PrintStudent(s);
    }

    std::cout << "\n=== 集計(std::accumulate) ===" << std::endl;
    std::cout << "合計点: " << SumScores(students) << std::endl;
    std::cout << "平均点: " << AverageScore(students) << std::endl;

    constexpr int kThreshold = 80;
    std::cout << "\n=== " << kThreshold
              << "点以上でフィルタリング(std::count_if / std::copy_if + ラムダ) ===" << std::endl;
    std::cout << kThreshold << "点以上の人数: " << CountAtLeast(students, kThreshold) << std::endl;
    for (const auto& s : FilterAtLeast(students, kThreshold)) {
        PrintStudent(s);
    }

    std::cout << "\n=== 成績別グルーピング(std::map、キー順に走査) ===" << std::endl;
    for (const auto& [grade, group] : GroupByGrade(students)) {
        std::cout << grade << ": " << group.size() << "人" << std::endl;
    }

    std::cout << "\n=== 成績別人数(std::unordered_map、順序は保証されない) ===" << std::endl;
    const auto gradeCounts = CountByGrade(students);
    for (const char* grade : {"A", "B", "C", "D", "F"}) {
        const auto it = gradeCounts.find(grade);
        const int count = (it != gradeCounts.end()) ? it->second : 0;
        std::cout << grade << ": " << count << "人" << std::endl;
    }

    std::cout << "\n=== 名前検索(std::find_if + イテレータ) ===" << std::endl;
    for (const std::string& name : {"Eve", "Zoe"}) {
        const auto it = FindByName(students, name);
        if (it != students.end()) {
            std::cout << name << " が見つかりました -> ";
            PrintStudent(*it);
        } else {
            std::cout << name << " は見つかりませんでした(end()に到達)" << std::endl;
        }
    }

    return 0;
}
