#ifndef GRADE_JUDGMENT_H
#define GRADE_JUDGMENT_H

#include <string>

// 0〜100点を10点刻みでA〜Fに判定する。main()から切り出すことで単体テストできるようにする。
std::string JudgeGrade(int score);

#endif  // GRADE_JUDGMENT_H
