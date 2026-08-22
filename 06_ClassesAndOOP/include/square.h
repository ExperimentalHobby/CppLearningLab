#ifndef SQUARE_H
#define SQUARE_H

#include "rectangle.h"

// Squareは「一辺の長さが等しいRectangle」として実装する（多段継承の例）。
// Area()/Perimeter()はRectangleの実装をそのまま使い、Name()だけをオーバーライドする。
class Square : public Rectangle {
public:
    explicit Square(double side);

    std::string Name() const override;
};

#endif  // SQUARE_H
