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

## テスト

挨拶メッセージの組み立てロジック（`include/greeting.h`/`src/greeting.cpp`）を
[third_party/googletest](../third_party/googletest) を使って単体テストしている
（本リポジトリで最初にGoogleTestを導入した課題であり、以降の課題のテンプレートとなる）。

```sh
cmake --build build --target BuildEnvironmentSetupTests
.\build\test\Debug\BuildEnvironmentSetupTests.exe
```

## Visual Studio 2026 で開く

このリポジトリではCMakeを唯一のビルド定義とし、`.sln`/`.vcxproj` は手動作成・コミットしない
（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。
VS2026でCMakeプロジェクトを直接開発・デバッグするには以下のいずれかを使う。

**この2つは同じ結果に至る2通りの手順ではなく、生成される場所・形式が異なる別々の
ビルドフローである点に注意。** どちらか一方を選び、混在させない（例: 片方で生成した
ディレクトリをもう片方のビルドに使い回さない）。

| | フォルダを開く | ソリューションファイルを生成 |
|---|---|---|
| 使い方 | VS2026で「フォルダーを開く」→ `01_BuildEnvironmentSetup` を選択 | `cmake -S . -B build -G "Visual Studio 18 2026" -A x64` を実行後、生成された `.slnx` をVS2026で開く |
| 何が使われるか | 同梱の `CMakePresets.json`（`x64-debug`/`x64-release`） | コマンドで明示した `Visual Studio 18 2026` ジェネレーター |
| 内部のビルドシステム | Ninja（シングルコンフィグ） | MSBuild（マルチコンフィグ） |
| 出力先 | `out/build/x64-debug/` など（`.sln`/`.vcxproj`は生成されない） | `build/`（`BuildEnvironmentSetup.slnx`と `.vcxproj`一式が生成される。VS2026以降はCMakeが `.sln` ではなく新形式の `.slnx` を生成する） |

いずれの出力先も `.gitignore` 対象のためコミットされない。迷ったら**フォルダを開く**方を
使う（`CMakePresets.json`により構成が明示されており、他課題でも同じ手順で統一できるため）。

### 「ソリューションを開いてもソースが無い」場合

`.slnx`/`.sln` を生成する方式では、実プロジェクトの他に**CMakeが自動生成するメタターゲット**が
ソリューションに含まれる。ソースが入っているのは `BuildEnvironmentSetup` プロジェクトのみで、
他の2つはソースを持たないのが仕様。

| プロジェクト | 役割 | ソース |
|---|---|---|
| `ALL_BUILD` | 全プロジェクトを一括ビルドする空のダミー（既定の起動プロジェクトになっていることが多い） | 無し（仕様） |
| `ZERO_CHECK` | `CMakeLists.txt` の変更を検知してcmakeを再実行する内部プロジェクト | 無し（仕様） |
| `BuildEnvironmentSetup` | 実際のプログラム本体（`src/main.cpp` はここに入っている） | 有り |

ソリューションエクスプローラーを開いたときに強調表示される既定プロジェクトが `ALL_BUILD` の
ため中身が空に見えるだけで、ソースを追加し直す必要はない。`BuildEnvironmentSetup` プロジェクトを
展開して確認し、ビルド・デバッグする際はこれを右クリックして「スタートアッププロジェクトに設定」する。
