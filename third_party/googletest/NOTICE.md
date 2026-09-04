# GoogleTest

このディレクトリには [GoogleTest](https://github.com/google/googletest) 公式リポジトリの
`googletest/include/`・`googletest/src/`（C++単体テストフレームワーク本体。GoogleMock・
ドキュメント・サンプル・自己テストは含まない）をそのまま同梱している。

- バージョン: v1.15.2
- 取得元: https://github.com/google/googletest/releases/tag/v1.15.2
- ライセンス: BSD 3-Clause（同梱の [LICENSE](LICENSE) を参照）

## なぜvcpkg等のパッケージマネージャを使わないのか

本リポジトリの開発環境にはvcpkg/Conan等が導入されておらず、外部パッケージマネージャの
インストールも前提にしていない（ルート [README.md](../../README.md#guiライブラリの選定について)、
[third_party/sqlite3/NOTICE.md](../sqlite3/NOTICE.md) と同じ方針）。GoogleTestは
`include/`・`src/`をそのままコンパイルに含めるだけで動作するため、パッケージマネージャなしで
「vendoring（ソースを直接同梱）」するのが最も単純で再現性が高い。

## 利用方法

テストを追加する各課題では、`test/CMakeLists.txt` から相対パスでこのディレクトリを参照し、
静的ライブラリとしてビルドして利用する。

```cmake
set(GOOGLETEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/googletest)
add_library(gtest STATIC ${GOOGLETEST_DIR}/src/gtest-all.cc)
target_include_directories(gtest PUBLIC ${GOOGLETEST_DIR}/include)
target_include_directories(gtest PRIVATE ${GOOGLETEST_DIR})

add_library(gtest_main STATIC ${GOOGLETEST_DIR}/src/gtest_main.cc)
target_link_libraries(gtest_main PUBLIC gtest)
```

`gtest-all.cc` はGoogleTest本体の全ソースを1ファイルに束ねたもので、これと`gtest_main.cc`
（`main()`を提供）の2ファイルをコンパイルするだけでGoogleTestが使える。複数の課題フォルダから
同じソースを共有することで、GoogleTest本体（約1MB）の重複コミットを避けている。

## 単体テストの対象範囲について

本リポジトリのGUI(Win32 API)・ソケット通信・シリアル・USB等、実機・実環境に依存する部分は
自動テスト化が難しいため、GoogleTestでの単体テスト対象は**純粋ロジック**（パーサー、統計計算、
プロトコルのエンコード/デコード等）に限定する。詳細な方針は [CLAUDE.md](../../CLAUDE.md) の
「テスト駆動開発を意識すること」を参照。
