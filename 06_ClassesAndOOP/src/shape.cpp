#include "shape.h"

#include <cmath>

std::ostream& operator<<(std::ostream& os, const Shape& shape) {
    os << shape.Name() << "(面積=" << shape.Area() << ", 周囲長=" << shape.Perimeter() << ")";
    return os;
}

bool operator==(const Shape& lhs, const Shape& rhs) {
    constexpr double kEpsilon = 1e-9;
    return std::abs(lhs.Area() - rhs.Area()) < kEpsilon;
}
