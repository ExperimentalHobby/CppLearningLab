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

## 成果物イメージ
21番のSQLite版と同等のCRUD操作を、MySQLまたはPostgreSQLに対して行う
コマンドラインツール。
