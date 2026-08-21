#include <iostream>
#include <memory>
#include <vector>

#include "circle.h"
#include "rectangle.h"
#include "shape.h"
#include "square.h"

#ifdef _WIN32
// windows.h(wingdi.h)はGDIのRectangle()/Ellipse()等をグローバル名前空間に宣言しており、
// 自作のRectangleクラスと衝突する。NOGDIを定義してGDI関連の宣言を除外する。
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    // Shapeは抽象基底クラスのため値では持てず、std::unique_ptr<Shape>で保持する。
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(2.0));
    shapes.push_back(std::make_unique<Rectangle>(3.0, 4.0));
    shapes.push_back(std::make_unique<Square>(5.0));

    std::cout << "=== 多態性: 基底クラスポインタ経由でも派生クラスの実装が呼ばれる ===" << std::endl;
    double totalArea = 0.0;
    for (const auto& shape : shapes) {
        // operator<<の中でName()/Area()/Perimeter()という仮想関数が呼ばれ、
        // 実際の型(Circle/Rectangle/Square)ごとの実装が使われる。
        std::cout << *shape << std::endl;
        totalArea += shape->Area();
    }
    std::cout << "合計面積: " << totalArea << std::endl;

    std::cout << "\n=== 演算子オーバーロード(operator==): 面積の近似比較 ===" << std::endl;
    const Circle circleA(2.0);
    const Circle circleB(2.0);
    const Rectangle rectangle(4.0, 3.14159265358979323846 * 2.0 * 2.0 / 4.0);  // circleAとほぼ同じ面積
    std::cout << "circleA == circleB: " << std::boolalpha << (circleA == circleB) << std::endl;
    std::cout << "circleA == rectangle(面積近似): " << (circleA == rectangle) << std::endl;

    std::cout << "\n=== 多段継承: SquareはRectangleを継承している ===" << std::endl;
    const Square square(5.0);
    const Shape& squareAsShape = square;  // 派生クラスは基底クラス型として扱える
    std::cout << squareAsShape << std::endl;

    return 0;
}
