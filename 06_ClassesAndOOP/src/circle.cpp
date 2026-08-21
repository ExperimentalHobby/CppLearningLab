#include "circle.h"

#include <stdexcept>

namespace {
// C++17時点では標準の円周率定数(<numbers>、C++20)が使えないため、自前で定義する。
constexpr double kPi = 3.14159265358979323846;
}  // namespace

Circle::Circle(double radius) : radius_(radius) {
    if (radius <= 0.0) {
        throw std::invalid_argument("radius must be positive");
    }
}

double Circle::Area() const {
    return kPi * radius_ * radius_;
}

double Circle::Perimeter() const {
    return 2.0 * kPi * radius_;
}

std::string Circle::Name() const {
    return "Circle";
}
