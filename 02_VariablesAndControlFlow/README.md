# 02. 変数・型・制御構造

## 目的
C++の基本文法（変数、型、演算子、条件分岐、ループ）を一通り復習し、
モダンC++での書き方（`auto`、範囲for、`constexpr`など）に慣れる。

## 学習ポイント
- 基本型・型推論(`auto`)・`const`/`constexpr`
- if/switch、for/while/範囲for
- 文字列操作（`std::string`）
- 簡単な入出力（`std::cin`/`std::cout`）

## 推奨ライブラリ/ツール
- 標準ライブラリのみ（`<string>`, `<iostream>` 等）

## 成果物イメージ
- FizzBuzzや簡単な成績判定など、制御構造を使う小問題を複数解く
- 数値当てゲーム（ユーザー入力とループ処理の練習）

## ビルド方法

01と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。
1つの `CMakeLists.txt` で3つの実行ファイルをビルドする構成になっている。

```sh
# 02_VariablesAndControlFlow ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
```

VS2026で開発する場合は「フォルダーを開く」で `CMakePresets.json` が自動検出される。

## 実行方法

| 実行ファイル | 内容 |
|---|---|
| `FizzBuzz` | 1〜30を`std::vector`+範囲forで走査し、FizzBuzzのルールで出力する |
| `GradeJudgment` | 5人分の点数を`switch`（フォールスルーあり）で成績(A〜F)に判定する |
| `NumberGuessingGame` | 1〜100の数値当てゲーム。標準入力から予想を読み取り、10回以内に正解を目指す |

```sh
# ビルド後、out/build/x64-debug/ 配下で実行
./out/build/x64-debug/FizzBuzz
./out/build/x64-debug/GradeJudgment
./out/build/x64-debug/NumberGuessingGame
```

`NumberGuessingGame` は通常は乱数デバイスでシードするが、動作確認用に
コマンドライン引数で乱数シードを指定できる（例: `./NumberGuessingGame 42`）。
