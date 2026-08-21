#ifndef STUDENT_H
#define STUDENT_H

#include <string>

struct Student {
    std::string name;
    int score;
};

// 0〜100点を10点刻みでA〜Fに判定する（02課題のGradeJudgmentと同様のルール）。
std::string DetermineGrade(int score);

#endif  // STUDENT_H
