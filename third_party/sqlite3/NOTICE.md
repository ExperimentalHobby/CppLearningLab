# SQLite Amalgamation

このディレクトリには [SQLite](https://www.sqlite.org/) 公式サイトで配布されている
「amalgamation」（`sqlite3.c`/`sqlite3.h` の2ファイルに全機能をまとめたソース配布形式）を
そのまま同梱している。

- バージョン: 3.45.1 (`sqlite-amalgamation-3450100`)
- 取得元: https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
- ライセンス: パブリックドメイン（https://www.sqlite.org/copyright.html）

## なぜvcpkg等のパッケージマネージャを使わないのか

本リポジトリの開発環境にはvcpkg/Conan等が導入されておらず、外部パッケージマネージャの
インストールも前提にしていない（ルート [README.md](../../README.md#guiライブラリの選定について)
と同じ方針）。SQLiteはpublic domainのamalgamation形式で配布されており、`sqlite3.c`を
1ファイルとしてそのままコンパイルに含めるだけで動作するため、パッケージマネージャなしで
「vendoring（ソースを直接同梱）」するのが最も単純で再現性が高い。

## 利用方法

21番以降のDB連携課題では、各課題の`CMakeLists.txt`から相対パスでこのディレクトリを
参照し、`sqlite3.c`を静的ライブラリとしてビルドして利用する。

```cmake
set(SQLITE3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/sqlite3)
add_library(sqlite3 STATIC ${SQLITE3_DIR}/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${SQLITE3_DIR})
```

複数の課題フォルダから同じソースを共有することで、amalgamation本体（約9MB）の
重複コミットを避けている。
