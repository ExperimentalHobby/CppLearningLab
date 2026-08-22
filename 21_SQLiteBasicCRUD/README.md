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

本課題では**SQLite3のamalgamation(`sqlite3.h`直接利用)**を採用する。開発環境に
vcpkg等のパッケージマネージャが無いため、[third_party/sqlite3](../third_party/sqlite3/NOTICE.md)
にvendoringした`sqlite3.c`/`sqlite3.h`を各課題のCMakeLists.txtから相対パスで
参照してビルドする（詳細はルート [README.md](../README.md#db連携21-27番台で採用したライブラリ環境上の制約)）。

## 成果物イメージ
簡単なTODOリストやメモをSQLiteに保存するコマンドラインツール
（追加・一覧表示・更新・削除ができる）。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 21_SQLiteBasicCRUD ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/SQLiteBasicCRUD [DBファイルパス(省略時 todo.db)]
```

起動後、標準入力からコマンドを入力してTODOを操作する。

```
add <タイトル>          新しいTODOを追加
list                    一覧表示
done <id>               完了にする
update <id> <タイトル>  タイトルを変更
remove <id>             削除
help                    このヘルプを表示
quit / exit             終了
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `TodoRepository::Open`/`Close`/デストラクタ | `sqlite3_open`/`sqlite3_close`をRAIIで管理 | SQLiteのDB接続・切断 |
| `TodoRepository::EnsureSchema` | `CREATE TABLE IF NOT EXISTS todos` | テーブル作成 |
| `TodoRepository::Add`/`List`/`FindById`/`Update`/`SetDone`/`Remove` | `sqlite3_prepare_v2`+バインドAPIによるINSERT/SELECT/UPDATE/DELETE | INSERT/SELECT/UPDATE/DELETE、結果セットの取得とC++構造体へのマッピング |
| `TodoItem`構造体 | SELECT結果を`id`/`title`/`done`を持つ構造体に変換 | 結果セットの取得とC++の構造体へのマッピング |

生SQLを文字列連結で組み立てず、常に`?`プレースホルダ+バインドを使っている点は
[23_PreparedStatementSecurity](../23_PreparedStatementSecurity/README.md)で扱う
SQLインジェクション対策の実践を先取りしたもの。

## 動作確認

ビルド・実行し、一時DBファイルに対して以下のコマンド列を標準入力から流し込んで確認済み。

```
add 牛乳を買う
add レポートを書く
list       -> [ ] #1 牛乳を買う / [ ] #2 レポートを書く
done 1
update 2 レポートを提出する
list       -> [x] #1 牛乳を買う / [ ] #2 レポートを提出する
remove 1
list       -> [ ] #2 レポートを提出する
```

追加・一覧表示・完了フラグ更新・タイトル更新・削除の一連の操作が期待通りに
反映されること、日本語のタイトルが文字化けせず保存・表示されることを確認した。
