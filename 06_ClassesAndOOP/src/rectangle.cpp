#include "rectangle.h"

#include <stdexcept>

Rectangle::Rectangle(double width, double height) : width_(width), height_(height) {
    if (width <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("width and height must be positive");
    }
}

double Rectangle::Area() const {
    return width_ * height_;
}

double Rectangle::Perimeter() const {
    return 2.0 * (width_ + height_);
}

std::string Rectangle::Name() const {
    return "Rectangle";
}
