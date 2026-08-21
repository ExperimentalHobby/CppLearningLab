# 06. クラス設計とOOP

## 目的
クラス・継承・多態性（仮想関数）といったオブジェクト指向の基礎を身につけ、
以降のGUI/DB課題で使うクラス設計の土台を作る。

## 学習ポイント
- クラス、アクセス指定子、コンストラクタ/デストラクタ
- 継承と仮想関数によるポリモーフィズム
- 抽象基底クラス、インターフェース設計
- 演算子オーバーロード

## 推奨ライブラリ/ツール
- 標準ライブラリのみ

## 成果物イメージ
図形クラス（`Shape`基底クラスと`Circle`/`Rectangle`等の派生クラス）で、
面積計算を仮想関数で多態的に呼び出すプログラム。

## ビルド方法

01〜05と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 06_ClassesAndOOP ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/ClassesAndOOP
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/shape.h`/`src/shape.cpp` | 抽象基底クラス`Shape`（純粋仮想関数、仮想デストラクタ）、`operator<<`/`operator==` | クラス、抽象基底クラス、演算子オーバーロード |
| `include/circle.h`/`src/circle.cpp` | `Circle : public Shape`（コンストラクタでの検証） | 継承、コンストラクタ |
| `include/rectangle.h`/`src/rectangle.cpp` | `Rectangle : public Shape`（`protected`アクセス指定子） | アクセス指定子 |
| `include/square.h`/`src/square.cpp` | `Square : public Rectangle`（コンストラクタ委譲、多段継承） | 継承の応用 |
| `src/main.cpp` | `std::vector<std::unique_ptr<Shape>>`で多態的に処理、`operator==`のデモ | 仮想関数によるポリモーフィズム |

## 補足: `windows.h`とクラス名の衝突について

`windows.h`(`wingdi.h`)はGDIの`Rectangle()`/`Ellipse()`等をグローバル名前空間に宣言しており、
自作の`Rectangle`クラスと名前が衝突する。`main.cpp`で`windows.h`をインクルードする前に
`NOGDI`（および`WIN32_LEAN_AND_MEAN`）を定義してGDI関連の宣言を除外することで回避している。
`Rectangle`/`Ellipse`/`Polygon`など図形系の名前をWindows環境で使う場合は同様の対策が必要になる。
