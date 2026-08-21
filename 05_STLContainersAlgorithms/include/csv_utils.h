#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include <istream>
#include <vector>

#include "student.h"

// "name,score" 形式のCSVを読み込み、Studentのvectorに変換する。
// 1行目はヘッダーとして読み飛ばす。数値に変換できない行は無視する。
std::vector<Student> LoadStudentsFromCsv(std::istream& input);

#endif  // CSV_UTILS_H
