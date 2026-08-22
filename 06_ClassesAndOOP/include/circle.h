#ifndef CIRCLE_H
#define CIRCLE_H

#include "shape.h"

class Circle : public Shape {
public:
    explicit Circle(double radius);

    double Area() const override;
    double Perimeter() const override;
    std::string Name() const override;

private:
    double radius_;
};

#endif  // CIRCLE_H
