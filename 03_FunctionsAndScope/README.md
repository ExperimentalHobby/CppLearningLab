# 03. 関数・スコープ・ヘッダー分割

## 目的
関数の設計（引数の渡し方、戻り値、オーバーロード）と、ヘッダー/ソース分割による
複数コンパイル単位のビルドを理解する。

## 学習ポイント
- 値渡し・参照渡し・constの使い分け
- 関数オーバーロード、デフォルト引数
- ヘッダーファイル(.h/.hpp)とソースファイル(.cpp)の分割、インクルードガード
- 複数.cppファイルからなるCMakeプロジェクトのビルド

## 推奨ライブラリ/ツール
- 標準ライブラリのみ、CMake

## 成果物イメージ
簡単な数学ユーティリティ関数群（最大公約数、素数判定など）をヘッダー/ソースに分割して実装し、
`main.cpp` から呼び出すミニライブラリ。

## ビルド方法

01/02と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。
`include/math_utils.h` + `src/math_utils.cpp` + `src/main.cpp` の複数コンパイル単位から
1つの実行ファイルをビルドする構成になっている。

```sh
# 03_FunctionsAndScope ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/FunctionsAndScope
```

VS2026で開発する場合は「フォルダーを開く」で `CMakePresets.json` が自動検出される。

## 構成

| ファイル | 内容 |
|---|---|
| `include/math_utils.h` | 関数宣言（インクルードガード、`math_utils`名前空間） |
| `src/math_utils.cpp` | 実装（GCD/LCM、素数判定、`Clamp`のオーバーロード、デフォルト引数、参照渡し、const参照渡し） |
| `src/main.cpp` | 各関数の呼び出しと結果表示。値渡し/参照渡し/constの違いが分かる出力になっている |

## 学習ポイントとの対応

| 学習ポイント | 実装箇所 |
|---|---|
| 値渡し | `Gcd`/`Lcm`/`IsPrime`（引数のコピーを受け取る） |
| 参照渡し | `Swap(int&, int&)`（呼び出し元の変数を書き換える） |
| const参照渡し | `SumOfDigits(const std::string&)`（コピーせず読み取り専用で渡す） |
| 関数オーバーロード | `Clamp(int,...)` と `Clamp(double,...)` |
| デフォルト引数 | `Power(int base, int exponent = 2)` |
| ヘッダー/ソース分割・インクルードガード | `math_utils.h`（`#ifndef`/`#define`） / `math_utils.cpp` |
| 複数.cppからなるCMakeビルド | `CMakeLists.txt` で `main.cpp` + `math_utils.cpp` を1つの実行ファイルに |
