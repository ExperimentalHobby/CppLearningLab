#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "shape.h"

class Rectangle : public Shape {
public:
    Rectangle(double width, double height);

    double Area() const override;
    double Perimeter() const override;
    std::string Name() const override;

protected:
    // Squareがコンストラクタ委譲以外で幅・高さに触れることはないため、
    // 本来はprivateでも十分だが、将来の派生クラス拡張を見越してprotectedにしている。
    double width_;
    double height_;
};

#endif  // RECTANGLE_H
