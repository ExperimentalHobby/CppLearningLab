# 22. MySQL/PostgreSQL接続

## 目的
サーバー型DB（MySQLまたはPostgreSQL）への接続とCRUD操作を実装し、
組み込みDBとの違い（接続情報、ネットワーク越しのアクセス）を理解する。

## 学習ポイント
- 接続文字列、ホスト/ポート/認証情報の扱い
- クライアントライブラリの利用（接続確立、クエリ実行、結果取得）
- 接続失敗時のエラーハンドリング

## 推奨ライブラリ/ツール
- MySQL: `libmysqlclient` / Connector/C++
- PostgreSQL: `libpqxx`
- (任意) DockerでローカルにMySQL/PostgreSQLを立てて動作確認

本課題では**ODBC**（Windows SDK標準の`odbc32.lib`/`odbccp32.lib`、追加インストール
不要）を採用する。採用理由と本開発環境での制約はルート
[README.md](../README.md#mysqlpostgresql接続22番はodbc--windows標準ライブラリを使用)
を参照。MySQL/PostgreSQLはいずれも公式ODBCドライバを提供しており、
同じODBC API(`SQLDriverConnect`/`SQLExecDirect`/`SQLFetch`等)でどちらにも
接続できるため、「サーバー型DBへの接続」という学習目的自体は満たせる。

## 成果物イメージ
21番のSQLite版と同等のCRUD操作を、MySQLまたはPostgreSQLに対して行う
コマンドラインツール。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 22_MySQLConnection ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/MySQLConnection <mysql|postgres> <host> <port> <database> <user> <password> [driver名]
```

`port`に`0`を指定すると、DBの種類に応じた既定ポート(MySQL: 3306, PostgreSQL: 5432)
が使われる。`driver名`を省略すると、それぞれ`MySQL ODBC 9.0 Unicode Driver`/
`PostgreSQL Unicode`という一般的なドライバ名を既定値として使う（実際にインストール
されているドライバ名と異なる場合は明示的に指定する）。

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `connection_string.h`/`.cpp`の`BuildOdbcConnectionString` | ホスト/ポート/DB名/認証情報からODBC接続文字列を組み立てる純粋関数(DB非依存) | 接続文字列、ホスト/ポート/認証情報の扱い |
| `OdbcConnection`(`odbc_connection.h`/`.cpp`) | `SQLAllocHandle`/`SQLDriverConnectW`/`SQLExecDirectW`/`SQLFetch`のRAIIラッパー | クライアントライブラリの利用(接続確立、クエリ実行、結果取得) |
| `OdbcConnection::ThrowError`/`FormatDiagnostics` | `SQLGetDiagRecW`で取得したSQLSTATE/ネイティブエラー/メッセージを整形して例外化 | 接続失敗時のエラーハンドリング |

ODBCのA系(ANSI)APIはドライバマネージャのエラーメッセージを実行環境のコードページ
(日本語環境ではShift-JIS)で返すため、UTF-8前提の本プログラムの出力と混在すると
文字化けする。そのため接続・クエリ実行・エラー取得は全てW系(Unicode)APIを使い、
`WideCharToMultiByte`/`MultiByteToWideChar`でUTF-8と相互変換している
（[16_SimpleTextEditor](../16_SimpleTextEditor)のUTF-8⇔UTF-16変換と同じ考え方）。

## 動作確認・環境上の制約

開発環境にMySQL/PostgreSQLサーバー本体も対応するODBCドライバも導入されておらず、
追加インストールもできないため、**実際のサーバーに接続してのCRUD成功は確認できていない**。
確認できた範囲は以下の通り。

- ビルドが通ること
- `BuildOdbcConnectionString`が、ホスト/ポート省略時の既定値補完、パスワードに
  `;`/`{`/`}`を含む場合のエスケープを含めて正しい接続文字列を組み立てること
  （実行結果はパスワード部分をマスクして表示）
- 存在しないドライバ名/DSNへの接続を試みると、`SQLSTATE=IM002`
  (Data source name not found)を含む診断メッセージが**文字化けせず**表示され、
  エラーハンドリング経路(`SQLGetDiagRecW`→例外→呼び出し元での捕捉)が
  正しく機能すること

```
$ MySQLConnection.exe mysql db.example.com 0 sampledb appuser p@ssword
接続文字列: Driver={MySQL ODBC 9.0 Unicode Driver};Server=db.example.com;Port=3306;Database=sampledb;Uid=appuser;Pwd=****;
接続またはクエリの実行に失敗しました。
DB接続に失敗しました: SQLSTATE=IM002 NativeError=0 Message=[Microsoft][ODBC Driver Manager] データ ソース名および指定された既定のドライバーが見つかりません。
```

実際のMySQL/PostgreSQLサーバーで動作確認する場合は、Docker等でサーバーを起動し、
対応するODBCドライバ（[MySQL Connector/ODBC](https://dev.mysql.com/downloads/connector/odbc/)、
[psqlODBC](https://odbc.postgresql.org/)）をインストールした上で、本ツールに
実際のホスト/ポート/認証情報を渡すこと。`RunCrudDemo`(`src/main.cpp`)は
テーブル作成→INSERT→UPDATE→SELECT→DELETEの一連のCRUDを行う。
