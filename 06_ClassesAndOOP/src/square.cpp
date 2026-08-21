#include "square.h"

Square::Square(double side) : Rectangle(side, side) {}

std::string Square::Name() const {
    return "Square";
}
