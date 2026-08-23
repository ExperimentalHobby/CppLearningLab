# 24. トランザクション制御

## 目的
複数のDB操作をひとまとまりとして扱うトランザクションの仕組みを理解し、
commit/rollbackによるデータ整合性の担保ができるようになる。

## 学習ポイント
- トランザクションの開始・commit・rollback
- 例外発生時の自動rollback（RAIIパターンとの組み合わせ）
- 分離レベルの概念（触りだけでよい）

## 推奨ライブラリ/ツール
- 21/22で使用したDBライブラリのトランザクションAPI

本課題では21/23番と同じ**SQLite3**（[third_party/sqlite3](../third_party/sqlite3/NOTICE.md)の
amalgamationを共有利用）を使う。

## 成果物イメージ
「口座Aから口座Bへ送金する」処理を題材に、途中でエラーを起こした場合に
残高の整合性が崩れないことをトランザクションで保証するプログラム。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う（詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照）。

```sh
# 24_TransactionHandling ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/TransactionHandling
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `ScopedTransaction`(`scoped_transaction.h`/`.cpp`) | コンストラクタで`BEGIN`、`Commit()`で`COMMIT`、未コミットのままデストラクタに達したら自動`ROLLBACK` | トランザクションの開始・commit・rollback、例外発生時の自動rollback(RAIIパターン) |
| `BankLedger::Transfer` | `ScopedTransaction`のスコープ内で出金→入金を実行し、最後に`Commit()`。途中で例外が飛べば自動ロールバック | 複数のDB操作をひとまとまりとして扱うトランザクション |
| `BankLedger::Withdraw`/`Deposit` | 残高不足・入金先不在を業務エラーとして例外化し、`Transfer`側の自動ロールバックに委ねる | 分離レベルの概念(触りとして、1トランザクション=1つの一貫した状態遷移という考え方) |

`ScopedTransaction`は[04_PointersAndMemory](../04_PointersAndMemory)の`ScopedResource`や
[07_ExceptionHandling](../07_ExceptionHandling)で扱った「例外が発生してもRAIIで後始末を
保証する」パターンをトランザクションに応用したもの。

## 動作確認

ビルド・実行し、以下の3ケースを確認済み。初期残高はAlice=1000, Bob=500。

| ケース | 内容 | 結果 |
|---|---|---|
| 1 | Alice→Bobに300送金(正常) | 成功しコミット。Alice=700, Bob=800 |
| 2 | Bob→Aliceに10000送金(残高不足) | `Withdraw`が例外を投げ自動ロールバック。残高はケース1直後のまま変化なし |
| 3 | Aliceから存在しない口座id=999へ100送金 | `Withdraw`(出金)はDB上で一度実行済みだが、後続の`Deposit`が入金先不在を検知して例外を投げ、コミット前に自動ロールバック。Aliceの残高はケース1直後のまま変化なし |

ケース3は「1つの操作は成功したが後続の操作が失敗した」場合でも、
トランザクション全体がロールバックされ、部分的な更新が残らないことを
示す最も重要なケース。`sqlite3_changes()`で影響行数を確認し、存在しない
口座IDへのUPDATE(SQL的にはエラーにならず0行更新で成功する)を明示的に
業務エラーとして扱っている点がポイント。
