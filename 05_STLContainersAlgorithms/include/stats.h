#ifndef STATS_H
#define STATS_H

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "student.h"

// 点数の降順でソートした新しいvectorを返す（std::sort + ラムダ）。
std::vector<Student> SortByScoreDescending(const std::vector<Student>& students);

// scoreがthreshold以上の人数を数える（std::count_if + ラムダ）。
std::size_t CountAtLeast(const std::vector<Student>& students, int threshold);

// scoreがthreshold以上の学生だけを集めた新しいvectorを返す（std::copy_if + ラムダ）。
std::vector<Student> FilterAtLeast(const std::vector<Student>& students, int threshold);

// 点数の合計を返す（std::accumulate）。
long long SumScores(const std::vector<Student>& students);

// 点数の平均を返す（studentsが空の場合は0.0）。
double AverageScore(const std::vector<Student>& students);

// 成績(A〜F)ごとに学生をグループ分けする（std::map: キー順に走査したい場合に使う）。
std::map<std::string, std::vector<Student>> GroupByGrade(const std::vector<Student>& students);

// 成績(A〜F)ごとの人数を数える（std::unordered_map: 順序を気にせず高速に集計したい場合に使う）。
std::unordered_map<std::string, int> CountByGrade(const std::vector<Student>& students);

// 名前で学生を検索する（std::find_ifとイテレータ）。
// 見つからなければstudents.end()と等しいイテレータを返す（呼び出し側で比較して判定する）。
std::vector<Student>::const_iterator FindByName(const std::vector<Student>& students,
                                                  const std::string& name);

#endif  // STATS_H
