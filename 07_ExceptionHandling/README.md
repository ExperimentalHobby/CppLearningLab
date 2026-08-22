# 07. 例外処理

## 目的
C++の例外処理機構(try/catch/throw)を理解し、RAIIと組み合わせた
安全なエラーハンドリングの設計ができるようになる。

## 学習ポイント
- try/catch/throw の基本
- 標準例外クラス階層(`std::exception`とその派生)
- 独自例外クラスの設計
- 例外安全性とRAII（デストラクタでの後始末が保証される仕組み）
- 例外 vs エラーコードの使い分けの考え方

## 推奨ライブラリ/ツール
- 標準ライブラリ（`<stdexcept>`, `<exception>`）

## 成果物イメージ
ファイル読み込みや数値変換など「失敗しうる処理」を含む小さなプログラムを作り、
独自例外クラスで意味のあるエラー情報を呼び出し側に伝える設計にする。

## ビルド方法

01〜06と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。
サンプル設定ファイル（`data/*.config`）へのパスはCMakeの`DATA_DIR`コンパイル定義で
絶対パスとして埋め込んでいるため、実行ディレクトリを気にせず動作する。

```sh
# 07_ExceptionHandling ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/ExceptionHandling
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/app_errors.h`/`src/app_errors.cpp` | `AppError`(基底)と`FileOpenError`/`ParseError`/`ValueOutOfRangeError`の独自例外階層 | 標準例外クラス階層、独自例外クラスの設計 |
| `include/scoped_trace.h`/`src/scoped_trace.cpp` | コンストラクタ/デストラクタでログを出すRAIIクラス | 例外安全性とRAII |
| `include/config_parser.h`/`src/config_parser.cpp` | 設定ファイルのパース・数値変換・範囲検証。`std::invalid_argument`を独自例外に変換して投げ直す | try/catch/throwの基本、例外の変換 |
| `src/main.cpp` | 5パターン（正常系/ファイル無し/構文エラー/数値変換エラー/範囲外エラー）を実行 | catch節の並び順（派生型→基底型） |
| `data/*.config` | 各パターン用のサンプル設定ファイル | - |

## 例外 vs エラーコードの使い分けについて

このプログラムでは「失敗が呼び出し階層の途中(`ParseConfigFile`の中の`ParsePositiveInt`や
`ValidatePort`)で起きても、最終的な呼び出し元(`main`)まで自動的に伝播してほしい」ため
例外を採用している。エラーコード（戻り値で成否を返す方式）だと、各呼び出し階層で
逐一チェックして早期returnで伝播させるコードが必要になり、本質的な処理（設定の解釈）が
エラー処理コードに埋もれやすい。一方で、ホットパスで頻繁に失敗しうる処理（例:
1件ずつのバリデーションを大量に流すループ）では、例外の送出コストが無視できない場合もあり、
エラーコードや`std::optional`/`std::expected`(C++23)的な戻り値の方が適することもある。
