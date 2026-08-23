# 26. 顧客管理アプリ（GUI+DB統合）

## 目的
GUI課題（11-18番）とDB課題（21-25番）で学んだ内容を統合し、
実用的なGUI+DBアプリケーションを構築する集大成的な課題。

## 学習ポイント
- GUIからのユーザー入力をDB操作に橋渡しする設計（UI層とデータ層の分離）
- 一覧表示（テーブルビュー）、検索、追加/編集/削除といったCRUD UIの実装
- 25番で作った簡易ORMラッパーの活用（任意）

## 推奨ライブラリ/ツール
- Qt6 (`QTableView`/`QTableWidget`) + SQLiteまたはMySQL

本課題ではGUIは11-18番と同じ**Win32 API**（一覧表示は`SysListView32`、
`comctl32.lib`）、DBは21番と同じ**SQLite3**
（[third_party/sqlite3](../third_party/sqlite3/NOTICE.md)のamalgamationを
共有利用）を採用する。理由はルート
[README.md](../README.md#guiライブラリの選定について)を参照。

## 成果物イメージ
顧客の氏名・連絡先などを一覧表示し、検索・追加・編集・削除ができる
シンプルな顧客管理デスクトップアプリ。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 26_CustomerManagementApp ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/CustomerManagementApp
```

実行ディレクトリに`customers.db`が作成され、顧客データが永続化される。

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `CustomerRepository`(`customer_repository.h`/`.cpp`、Win32非依存) | `customers`テーブルのCRUD+氏名の部分一致検索(`LIKE`、ワイルドカード文字はエスケープ) | GUIからのユーザー入力をDB操作に橋渡しする設計(UI層とデータ層の分離) |
| `SysListView32`(レポートビュー、4列)+ 各行`lParam`への顧客id格納 | 一覧表示(テーブルビュー) | 一覧表示、検索、追加/編集/削除といったCRUD UIの実装 |
| `OnAdd`/`OnUpdate`/`OnDelete`/`OnSelectionChanged` | ボタン押下・行選択に応じたUI⇔`CustomerRepository`の橋渡し | CRUD UIの実装 |

25番の`Repository<T, Mapper>`は今回は使わず、21番の`TodoRepository`と同じ
直接的なリポジトリクラスにした(顧客1エンティティのみでテーブルも1つのため、
ジェネリックな抽象化を導入するメリットが薄いと判断)。

## 動作確認

- `CustomerRepository`単体は、21番と同様に一時的な検証用コンソールプログラム
  (リポジトリには含めていない)で検証: 追加(id自動採番)/一覧/氏名の部分一致検索
  (一致あり・なし)/更新(反映確認)/削除(反映確認・存在しないidの削除がfalseを
  返すこと)を確認
- GUIをビルド・実行し、`EnumWindows`+`GetWindowThreadProcessId`でプロセスの
  トップレベルウィンドウを取得した上で`EnumChildWindows`し、
  氏名/電話番号/メール/検索の各`Edit`、追加/更新/削除/クリア/検索/全件表示の
  各`Button`、`SysListView32`が期待通りのコントロールID付きで生成されていることを
  構造的に確認
- 氏名/電話番号/メール欄に`WM_CHAR`で実際のキー入力を模擬し、追加ボタンを
  `BM_CLICK`したところ、`SysListView32`の件数(`LVM_GETITEMCOUNT`、スカラー値の
  ため読み取りが安定している)が0→1に増加することを確認。さらにプロセス終了後、
  作成された`customers.db`を別プロセスから読み込み、入力した氏名・電話番号・
  メールアドレスがそのまま保存されていることを確認し、GUI操作からDB永続化までの
  一連の流れが実際に機能することを検証した

補足: `FindWindow`によるウィンドウ検索は本開発環境では失敗する(戻り値0)ため、
`EnumWindows`+`GetWindowThreadProcessId`でプロセスIDから対象ウィンドウを
特定する方式に切り替えている。子コントロールへの`WM_SETTEXT`によるテキスト
読み取りはこれまでの課題(12番等)と同様に不安定だったため、本課題では
「入力→操作→件数のスカラー値変化→DBファイルの内容」という経路で検証している。
