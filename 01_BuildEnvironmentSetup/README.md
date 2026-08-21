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

## ビルド方法（コマンドライン）

out-of-source build（ビルド専用ディレクトリ `build/` を分離する方式）でビルドする。

```sh
# 01_BuildEnvironmentSetup ディレクトリで実行
cmake -S . -B build
cmake --build build

# 実行（Windows/MSVCの場合）
.\build\Debug\BuildEnvironmentSetup.exe

# 実行（Linux/macOS、またはMinGWの場合）
./build/BuildEnvironmentSetup
```

`Hello, C++ Learning Lab!` と表示されれば成功。`build/` は `.gitignore` 対象なので
コミットされない。

## Visual Studio 2026 で開く

このリポジトリではCMakeを唯一のビルド定義とし、`.sln`/`.vcxproj` は手動作成・コミットしない
（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。
VS2026でCMakeプロジェクトを直接開発・デバッグするには以下のいずれかを使う。

- **フォルダを開く**: VS2026のスタート画面で「フォルダーを開く」からこの
  `01_BuildEnvironmentSetup` フォルダを選択する。同梱の `CMakePresets.json` が
  自動検出され、`x64-debug` / `x64-release` の構成が選択できる状態でビルド・
  デバッグ実行が可能。
- **ソリューションファイルを生成する**（従来型のプロジェクト管理をしたい場合）:
  ```sh
  cmake -S . -B build -G "Visual Studio 18 2026" -A x64
  ```
  生成された `build/BuildEnvironmentSetup.sln` をVS2026で開く。生成物は
  `.gitignore` 対象のためコミットされない。
