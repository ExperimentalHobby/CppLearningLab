#include "csv_utils.h"

#include <sstream>
#include <stdexcept>
#include <string>

std::vector<Student> LoadStudentsFromCsv(std::istream& input) {
    std::vector<Student> students;
    std::string line;

    bool isHeader = true;
    while (std::getline(input, line)) {
        if (isHeader) {
            isHeader = false;
            continue;
        }
        if (line.empty()) {
            continue;
        }

        const auto commaPos = line.find(',');
        if (commaPos == std::string::npos) {
            continue;  // 不正な行はスキップする
        }

        Student student;
        student.name = line.substr(0, commaPos);
        try {
            student.score = std::stoi(line.substr(commaPos + 1));
        } catch (const std::exception&) {
            continue;  // 数値に変換できない行はスキップする
        }

        students.push_back(std::move(student));
    }

    return students;
}
