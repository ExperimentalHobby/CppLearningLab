# C++ Learning Lab

C++の基礎からGUIプログラミング・DB連携・通信・USBデバイス制御までを段階的に学ぶための
学習リポジトリです。全38課題をルート直下にフラットに配置し、番号帯でカテゴリを表現しています。
各課題は独立したフォルダになっており、フォルダ内の `README.md` に目的・学習ポイント・
推奨ライブラリ・成果物イメージを記載しています。

## 番号帯とカテゴリ

カテゴリごとに10番台ずつ帯を空けているので、後から課題を追加してもリナンバーが不要です
（例: 基礎を9個目まで増やしても09が空いているので衝突しません）。

| 番号帯 | カテゴリ | 課題数 | 内容 |
|--------|----------|--------|------|
| 01-08 | 基礎 (Fundamentals) | 8 | 環境構築、文法、メモリ管理、STL、OOP、例外、テンプレート |
| 11-18 | GUIプログラミング | 8 | ウィンドウ表示からマルチスレッドGUIまで |
| 21-27 | DB連携 (Database) | 7 | SQLite/MySQL接続、CRUD、トランザクション、ORM |
| 31-38 | 通信 (Networking) | 8 | TCP/UDP、HTTP、WebSocket、シリアル通信 |
| 41-47 | USB | 7 | USBデバイス列挙、HID通信、ファイル転送 |

## 課題一覧

### 01-08 基礎
| # | フォルダ | 概要 |
|---|----------|------|
| 01 | [01_BuildEnvironmentSetup](01_BuildEnvironmentSetup/README.md) | CMakeでHello Worldをビルド・実行する |
| 02 | [02_VariablesAndControlFlow](02_VariablesAndControlFlow/README.md) | 変数・型・制御構造の基本文法 |
| 03 | [03_FunctionsAndScope](03_FunctionsAndScope/README.md) | 関数・スコープ・ヘッダー分割 |
| 04 | [04_PointersAndMemory](04_PointersAndMemory/README.md) | ポインタ・参照・スマートポインタ・RAII |
| 05 | [05_STLContainersAlgorithms](05_STLContainersAlgorithms/README.md) | STLコンテナとアルゴリズム |
| 06 | [06_ClassesAndOOP](06_ClassesAndOOP/README.md) | クラス設計・継承・多態性 |
| 07 | [07_ExceptionHandling](07_ExceptionHandling/README.md) | 例外処理 |
| 08 | [08_TemplatesGenericProgramming](08_TemplatesGenericProgramming/README.md) | テンプレート・ジェネリックプログラミング |

### 11-18 GUIプログラミング
| # | フォルダ | 概要 |
|---|----------|------|
| 11 | [11_HelloWindow](11_HelloWindow/README.md) | 空のウィンドウを1枚表示する |
| 12 | [12_BasicWidgets](12_BasicWidgets/README.md) | ボタン・ラベル・レイアウト・メニュー/ツールバー/ステータスバー |
| 13 | [13_EventHandling](13_EventHandling/README.md) | イベント/シグナル・スロットの仕組み |
| 14 | [14_DialogsAndMessageBox](14_DialogsAndMessageBox/README.md) | モーダル/モードレスダイアログ、メッセージボックス |
| 15 | [15_CalculatorApp](15_CalculatorApp/README.md) | 状態管理を伴うGUI電卓 |
| 16 | [16_SimpleTextEditor](16_SimpleTextEditor/README.md) | ファイル入出力と連携する簡易テキストエディタ |
| 17 | [17_PaintApp](17_PaintApp/README.md) | 描画APIとマウスイベントを使う簡易お絵かきアプリ |
| 18 | [18_MultiThreadedGUI](18_MultiThreadedGUI/README.md) | 進捗バー・非同期処理とUIスレッドの分離 |

### 21-27 DB連携
| # | フォルダ | 概要 |
|---|----------|------|
| 21 | [21_SQLiteBasicCRUD](21_SQLiteBasicCRUD/README.md) | SQLiteでの基本CRUD操作 |
| 22 | [22_MySQLConnection](22_MySQLConnection/README.md) | MySQL/PostgreSQLへの接続と操作 |
| 23 | [23_PreparedStatementSecurity](23_PreparedStatementSecurity/README.md) | プリペアドステートメントとSQLインジェクション対策 |
| 24 | [24_TransactionHandling](24_TransactionHandling/README.md) | トランザクション制御(commit/rollback) |
| 25 | [25_ORMWrapperDesign](25_ORMWrapperDesign/README.md) | 簡易ORM風ラッパークラスの設計 |
| 26 | [26_CustomerManagementApp](26_CustomerManagementApp/README.md) | GUI(15番台)と連携した顧客管理アプリ |
| 27 | [27_CSVImportExport](27_CSVImportExport/README.md) | CSVインポート/エクスポート |

### 31-38 通信
| # | フォルダ | 概要 |
|---|----------|------|
| 31 | [31_TCPClientServer](31_TCPClientServer/README.md) | TCPソケットのクライアント/サーバー通信 |
| 32 | [32_UDPCommunication](32_UDPCommunication/README.md) | UDP通信の基本 |
| 33 | [33_HTTPClientLibcurl](33_HTTPClientLibcurl/README.md) | libcurl等を使ったHTTPクライアント |
| 34 | [34_SimpleHTTPServer](34_SimpleHTTPServer/README.md) | 簡易HTTPサーバーの自作 |
| 35 | [35_SerialCommunication](35_SerialCommunication/README.md) | シリアル通信(RS-232/COMポート) |
| 36 | [36_MultiClientChatServer](36_MultiClientChatServer/README.md) | マルチスレッド/非同期チャットサーバー |
| 37 | [37_WebSocketCommunication](37_WebSocketCommunication/README.md) | WebSocketによる双方向通信 |
| 38 | [38_BinaryProtocolDesign](38_BinaryProtocolDesign/README.md) | 独自バイナリプロトコルの設計とパース |

### 41-47 USB
| # | フォルダ | 概要 |
|---|----------|------|
| 41 | [41_USBDeviceEnumeration](41_USBDeviceEnumeration/README.md) | libusb等でのUSBデバイス列挙 |
| 42 | [42_USBDeviceInfo](42_USBDeviceInfo/README.md) | デバイス情報(VID/PID等)の取得 |
| 43 | [43_HIDDataTransfer](43_HIDDataTransfer/README.md) | HIDデバイスとのデータ送受信 |
| 44 | [44_USBSerialCDC](44_USBSerialCDC/README.md) | USBシリアル(CDC)通信 |
| 45 | [45_MicrocontrollerUSBComm](45_MicrocontrollerUSBComm/README.md) | Arduino等マイコンとのUSB通信 |
| 46 | [46_USBHotplugDetection](46_USBHotplugDetection/README.md) | 抜き差し検知(WM_DEVICECHANGE等) |
| 47 | [47_USBFileTransferProtocol](47_USBFileTransferProtocol/README.md) | 独自プロトコルでのファイル転送 |

## 進め方の目安

1. **01-08 基礎** で言語機能とビルド環境を固める
2. **11-18 GUI** で画面に何かを表示できるようにする
3. **21-27 DB連携** でGUIアプリにデータ永続化を組み込む（26番でGUIと連携）
4. **31-38 通信** でアプリ間・デバイス間の通信を扱う
5. **41-47 USB** で物理デバイスとの連携に触れる（実機やUSBデバイスがない場合は仮想COMポートや
   エミュレータでの代替も検討）

カテゴリ内の課題は基本的に易しい順に並んでいますが、興味のあるテーマから進めても構いません。
課題数は固定ではないので、良い題材があれば各番号帯の空き番号に追加していく想定です。

## 推奨開発環境

- コンパイラ: MSVC (Visual Studio) または MinGW-w64 (g++)
- ビルドシステム: CMake
- GUIライブラリ: Qt6 / wxWidgets / Win32 API（課題ごとに選択）
- DBライブラリ: SQLite3, libmysqlclient, libpqxx など
- 通信ライブラリ: 標準ソケットAPI (Winsock2), libcurl, Boost.Asio など
- USBライブラリ: libusb, hidapi

各課題フォルダの `README.md` で個別に必要なライブラリを案内します。

## CMakeとVisual Studioの使い分け

全課題共通で、**CMake（`CMakeLists.txt`）をビルド定義の唯一の正とする**。
Visual Studioの `.sln`/`.vcxproj` は手動作成・コミットせず、以下のいずれかの方法で
CMake経由でVisual Studioを利用する。

- **フォルダを開く（推奨）**: 各課題フォルダをVisual Studioで「フォルダーを開く」。
  同梱の `CMakePresets.json`（`x64-debug` / `x64-release`）が自動検出され、
  ビルド・デバッグ実行ができる。内部ではNinjaが使われ、`out/build/x64-debug/` などに
  出力される（`.sln`/`.vcxproj`は生成されない）。
- **ソリューションファイルを生成する**: 従来型のプロジェクト管理をしたい場合は
  `cmake -S . -B build -G "Visual Studio 18 2026" -A x64` でその場で `.sln` を
  生成できる。内部ではMSBuildが使われ、`build/`に出力される。

上記2つは同じ結果に至る2通りの手順ではなく、**内部のビルドシステム（Ninja/MSBuild）も
出力先も異なる別々のフロー**である点に注意（片方の出力をもう片方で使い回さない）。
生成物はいずれも `.gitignore` 対象のためコミットされない。

この方針により、コマンドライン・VS Code (CMake Tools)・Visual Studioのどれでも
同じ `CMakeLists.txt` から一貫してビルドでき、プロジェクトファイルの二重メンテナンスを
避けられる。新しい課題を追加する際は、`CMakeLists.txt` に加えて `CMakePresets.json`
（[01_BuildEnvironmentSetup](01_BuildEnvironmentSetup/CMakePresets.json) をテンプレートとする）
も一緒に配置すること。

## 日本語を扱う際の文字コードの注意点

ソースコードは全てUTF-8で記述する。Windows(MSVC)環境では以下の2点を合わせて
対応しないと、日本語のコメントや出力文字列が文字化けする（[02_VariablesAndControlFlow](02_VariablesAndControlFlow)
で実例と対処済み）。

- **コンパイル時**: `CMakeLists.txt` でMSVC向けに `/utf-8` フラグを付ける
  （既定のコードページでソースを誤読され、日本語コメントを含む行以降が構文エラーになるのを防ぐ）。
  ```cmake
  if(MSVC)
      add_compile_options(/utf-8)
  endif()
  ```
- **実行時**: `main()` の先頭で `SetConsoleOutputCP(CP_UTF8)`（標準入力も扱うなら
  `SetConsoleCP(CP_UTF8)` も）を呼ぶ（`<windows.h>`、`_WIN32` でガード）。
  コンソールの既定コードページ(Shift-JIS等)のままだと、UTF-8で出力した日本語が
  文字化けする。これにより `chcp 65001` を事前に手動実行する必要がなくなる。
  ```cpp
  #ifdef _WIN32
  #include <windows.h>
  #endif

  int main() {
  #ifdef _WIN32
      SetConsoleOutputCP(CP_UTF8);
  #endif
      // ...
  }
  ```

## GUIライブラリの選定について

11〜18番台(GUIプログラミング)の各課題READMEは、当初の想定ライブラリとしてQt6/wxWidgetsを
挙げているが、本リポジトリでは**Win32 API**を採用する。

- 開発環境にQt6/wxWidgets/vcpkgが導入されておらず、追加インストールができない
- Win32 APIはVisual Studio同梱のツール（MSVC、Windows SDK）のみでビルドでき、
  01〜08と同じCMake構成をそのまま継続できる
- ネイティブWindows GUIプログラミングの基礎（ウィンドウ・メッセージループ・
  コモンコントロール・GDI・スレッド間通信）を学ぶこと自体に学習価値がある

Win32 API使用時の共通の注意点:

- ウィンドウ/コントロールを作る実行ファイルは`add_executable(... WIN32 ...)`で
  GUIサブシステムとしてビルドし、エントリポイントは`WinMain`/`wWinMain`にする
- `UNICODE`/`_UNICODE`を定義し、`W`サフィックスの関数(`CreateWindowExW`等)と
  `wchar_t`文字列(`L"..."`)を使う
- 独自クラス名が`windows.h`のグローバル関数と衝突することがある
  （[06_ClassesAndOOP](06_ClassesAndOOP/README.md) の`Rectangle`/GDI関数の衝突を参照）。
  `NOGDI`等のマクロで該当機能を除外するか、名前を変えて回避する
- コモンコントロール(ツールバー/ステータスバー/プログレスバー等)を使う課題は
  `comctl32.lib`を、共通ダイアログ(ファイル選択等)を使う課題は`comdlg32.lib`を
  `target_link_libraries`でリンクする

## `.rc`リソース(DIALOGEX等)を使う際の既知の問題

[14_DialogsAndMessageBox](14_DialogsAndMessageBox) で、`.rc`の`DIALOGEX`リソースを
`DialogBoxParam`/`CreateDialogParam`で読み込むと、本開発環境では
`GetLastError()=1814`（`ERROR_RESOURCE_NAME_NOT_FOUND`）で失敗する問題が発生し、
原因を特定できなかった（`rc.exe`単体でのコンパイルは成功し、`FindResourceW`で
直接調べても実行ファイル内に該当リソースが見当たらない。CMakeの自動マニフェスト
埋め込み(`/MANIFEST:NO`)を無効化しても改善しなかった）。

回避策として、ダイアログは`.rc`を使わず他の課題と同じ`CreateWindowExW`ベースの
ポップアップウィンドウとして実装している。今後の課題で`.rc`リソース
（アイコン、アクセラレータ、`DIALOGEX`等）を使う場合は、まず小さく検証してから
本格的に組み込むこと。

## GUIの外部自動化検証における`FindWindow`の既知の問題

[26_CustomerManagementApp](26_CustomerManagementApp)の動作確認で判明した問題として、
本開発環境では外部の自動化スクリプト(PowerShell)から`FindWindow`(クラス名・
ウィンドウタイトルどちらで検索しても)を呼ぶと、対象のウィンドウが実際に
存在し前面に表示されていても戻り値が常に`0`(見つからない)になる。

回避策として、`EnumWindows`で全トップレベルウィンドウを列挙し、
`GetWindowThreadProcessId`で対象プロセスIDと一致するものを探す方式に
切り替えたところ、正しくウィンドウハンドルを取得できた。以降のGUI課題で
外部スクリプトからウィンドウを特定する場合は、`FindWindow`ではなく
この`EnumWindows`ベースの方式を使うこと。

## DB連携(21-27番台)で採用したライブラリ・環境上の制約

### SQLiteはamalgamationをvendoring

開発環境にvcpkg/Conan等のパッケージマネージャが無いため、21・23・24・25・26・27番の
各課題では[third_party/sqlite3](third_party/sqlite3/NOTICE.md)に同梱した
SQLite公式のamalgamation(`sqlite3.c`/`sqlite3.h`の2ファイル、パブリックドメイン)を
各課題の`CMakeLists.txt`から相対パスで参照し、静的ライブラリとしてビルドする。
複数課題から同じソースを共有することで、amalgamation本体(約9MB)の重複コミットを
避けている。

### MySQL/PostgreSQL接続(22番)はODBC + Windows標準ライブラリを使用

22番の課題では、`libmysqlclient`/`libpqxx`等の専用クライアントライブラリが
本開発環境に無く追加インストールもできないため、**ODBC**（Windows SDKに標準で
含まれる`odbc32.lib`/`odbccp32.lib`、追加インストール不要）を接続層として採用した。
MySQL/PostgreSQLはいずれも公式のODBCドライバを提供しており、同じODBC API
(`SQLConnect`/`SQLExecDirect`/`SQLFetch`等)でどちらにも接続できる。

ただし本開発環境にはMySQL/PostgreSQLサーバー本体もODBCドライバも導入されておらず、
実際のサーバーに接続してのCRUD動作確認はできていない（詳細は
[22_MySQLConnection/README.md](22_MySQLConnection/README.md)の「動作確認」を参照）。
接続文字列の組み立てロジックと、接続失敗時のエラーハンドリング（`SQLGetDiagRec`
によるエラーメッセージ整形）は実際に検証済み。実サーバーで確認する場合は、
DockerでMySQL/PostgreSQLコンテナを起動し、対応するODBCドライバをインストールした上で
接続文字列を渡すこと。
