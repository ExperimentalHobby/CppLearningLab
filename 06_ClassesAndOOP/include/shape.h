#ifndef SHAPE_H
#define SHAPE_H

#include <ostream>
#include <string>

// 図形の抽象基底クラス（インターフェース）。
// Area()/Perimeter()/Name()を純粋仮想関数にすることで、Shape自体はインスタンス化できず、
// 派生クラスに実装を強制する。
class Shape {
public:
    virtual ~Shape() = default;

    virtual double Area() const = 0;
    virtual double Perimeter() const = 0;
    virtual std::string Name() const = 0;
};

// Shapeを基底クラスポインタ/参照経由で渡しても、Name()/Area()/Perimeter()は
// 仮想関数なので実際の派生クラスの実装が呼ばれる（多態性）。
std::ostream& operator<<(std::ostream& os, const Shape& shape);

// 面積がほぼ等しいかどうかを比較する（浮動小数点誤差を考慮した近似比較）。
bool operator==(const Shape& lhs, const Shape& rhs);

#endif  // SHAPE_H
