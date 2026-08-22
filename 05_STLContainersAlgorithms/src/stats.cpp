#include "stats.h"

#include <algorithm>
#include <iterator>
#include <numeric>

std::vector<Student> SortByScoreDescending(const std::vector<Student>& students) {
    std::vector<Student> sorted = students;
    std::sort(sorted.begin(), sorted.end(),
              [](const Student& a, const Student& b) { return a.score > b.score; });
    return sorted;
}

std::size_t CountAtLeast(const std::vector<Student>& students, int threshold) {
    return std::count_if(students.begin(), students.end(),
                          [threshold](const Student& s) { return s.score >= threshold; });
}

std::vector<Student> FilterAtLeast(const std::vector<Student>& students, int threshold) {
    std::vector<Student> result;
    std::copy_if(students.begin(), students.end(), std::back_inserter(result),
                 [threshold](const Student& s) { return s.score >= threshold; });
    return result;
}

long long SumScores(const std::vector<Student>& students) {
    return std::accumulate(students.begin(), students.end(), 0LL,
                            [](long long sum, const Student& s) { return sum + s.score; });
}

double AverageScore(const std::vector<Student>& students) {
    if (students.empty()) {
        return 0.0;
    }
    return static_cast<double>(SumScores(students)) / static_cast<double>(students.size());
}

std::map<std::string, std::vector<Student>> GroupByGrade(const std::vector<Student>& students) {
    std::map<std::string, std::vector<Student>> grouped;
    for (const auto& student : students) {
        grouped[DetermineGrade(student.score)].push_back(student);
    }
    return grouped;
}

std::unordered_map<std::string, int> CountByGrade(const std::vector<Student>& students) {
    std::unordered_map<std::string, int> counts;
    for (const auto& student : students) {
        ++counts[DetermineGrade(student.score)];
    }
    return counts;
}

std::vector<Student>::const_iterator FindByName(const std::vector<Student>& students,
                                                  const std::string& name) {
    return std::find_if(students.begin(), students.end(),
                         [&name](const Student& s) { return s.name == name; });
}
