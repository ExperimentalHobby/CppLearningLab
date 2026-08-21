# 01. 開発環境構築とビルド

## 目的
CMakeを使ったビルド環境を構築し、C++プロジェクトの「書く→ビルドする→実行する」という
一連の流れを体験する。以降すべての課題の土台となる。

## 学習ポイント
- コンパイラ(MSVC/g++)とCMakeのインストール・バージョン確認
- `CMakeLists.txt` の書き方（`add_executable`、C++標準バージョン指定）
- コマンドラインまたはVS CodeからのビルドとデバッグRun
- ビルドディレクトリの分離（out-of-source build）

## 推奨ライブラリ/ツール
- CMake 3.20+
- MSVC (Visual Studio) または MinGW-w64 (g++)
- (任意) VS Code + CMake Tools拡張

## 成果物イメージ
`cmake` でビルドし、コンソールに `Hello, C++ Learning Lab!` と表示するだけの
最小プログラム。ただし手打ちでなくCMake経由でビルドが通ることを確認するのがゴール。
