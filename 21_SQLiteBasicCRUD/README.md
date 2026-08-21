# 21. SQLite基本CRUD

## 目的
組み込みDBであるSQLiteを使い、C++からのCreate/Read/Update/Deleteの
基本操作を実装する。DB連携課題全体の入り口となる。

## 学習ポイント
- SQLiteのDB接続・切断
- テーブル作成、INSERT/SELECT/UPDATE/DELETE
- 結果セットの取得とC++の構造体/クラスへのマッピング

## 推奨ライブラリ/ツール
- SQLite3（`sqlite3.h` 直接利用、または軽量ラッパー `SQLiteCpp` 等）

## 成果物イメージ
簡単なTODOリストやメモをSQLiteに保存するコマンドラインツール
（追加・一覧表示・更新・削除ができる）。
