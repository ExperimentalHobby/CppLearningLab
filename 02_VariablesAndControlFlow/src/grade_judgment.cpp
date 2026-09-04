#include "grade_judgment.h"

// switchのフォールスルーで90点台と100点をまとめて"A"に判定している。
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
