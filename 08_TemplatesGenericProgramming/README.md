# 08. テンプレート・ジェネリックプログラミング

## 目的
関数テンプレート・クラステンプレートを使い、型に依存しない汎用的なコードを
書けるようになる。以降のGUI/DB課題で使うコンテナ・ラッパー設計の土台になる。

## 学習ポイント
- 関数テンプレートの基本
- クラステンプレート（テンプレートを使った汎用コンテナ設計）
- テンプレートの特殊化
- `concepts`（C++20）による制約（任意、余裕があれば）

## 推奨ライブラリ/ツール
- 標準ライブラリのみ

## 成果物イメージ
任意の型を格納できる固定長スタック/キューをテンプレートクラスとして実装し、
`int`や`std::string`など複数の型でインスタンス化して動作を確認する。

## ビルド方法

01〜07と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 08_TemplatesGenericProgramming ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/TemplatesGenericProgramming
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/fixed_stack.h`（ヘッダーオンリー） | `FixedStack<T, Capacity>`（`std::array`+固定長、容量超過/空アクセスで例外） | クラステンプレート |
| `include/fixed_queue.h`（ヘッダーオンリー） | `FixedQueue<T, Capacity>`（循環バッファ） | クラステンプレートの応用 |
| `include/display.h`/`src/display.cpp` | `Max<T>`（関数テンプレート）、`ToDisplayString<T>`の汎用版とbool/std::string向け明示的特殊化 | 関数テンプレートの基本、テンプレートの特殊化 |
| `src/main.cpp` | `FixedStack<int,4>`/`FixedStack<std::string,3>`/`FixedQueue<int,3>`など複数の型でインスタンス化して動作確認 | - |

`concepts`(C++20)による制約は学習ポイント上「任意（余裕があれば）」とされており、
本リポジトリは全課題でC++17を基準としているため、今回は導入していない。

## 補足: テンプレートの実装をヘッダーに書く理由

`FixedStack`/`FixedQueue`はテンプレートクラスであり、`FixedStack<int, 4>`のように
実際に使われた型・値でインスタンス化されて初めてコードが実体化する。03課題のように
宣言(.h)と実装(.cpp)を分離すると、その.cppをコンパイルする時点ではどの型で
使われるか分からずインスタンス化できないため、リンクエラーになる。そのため
クラステンプレート/汎用の関数テンプレートは定義ごとヘッダーに書く。

一方、`ToDisplayString`の`bool`/`std::string`向け明示的特殊化は対象の型が
確定しているため、通常の関数と同様に宣言をヘッダー、定義を`.cpp`に分離できる
（`src/display.cpp`を参照）。
