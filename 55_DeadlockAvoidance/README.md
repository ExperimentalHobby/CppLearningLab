# 55. デッドロックの発生と回避

## 目的
複数のmutexを異なる順序でロックすることで発生するデッドロックを実際に体験し、
`std::scoped_lock`等による回避方法を学ぶ。

## 学習ポイント
- 2つのmutexを異なる順序でロックする2スレッドを用意し、意図的にデッドロックを再現する
- ロック順序を統一する(常に同じ順でロックする)ことでの回避
- `std::scoped_lock`(C++17、複数mutexを原子的にロックしデッドロックを起こさない)による回避
- タイムアウト付きロック(`std::timed_mutex`/`try_lock_for`)による、デッドロックを検知して
  諦める方式の実演

## 採用ライブラリ/ツール
- 標準ライブラリのみ(`<mutex>`)

## デッドロックを試験する際の安全設計

デッドロックする可能性のあるコードを自動テストで検証する場合、素朴に
`std::thread::join()`で待つと、実際にデッドロックした際にテストプロセス自体が
永久に停止してしまう。これを避けるため、テストヘルパー`RunWithTimeout(work, timeout)`
を用意した。`work`をdetachした別スレッドで実行し、完了フラグを一定間隔で
ポーリングする。`detach()`しているため、`work`が万一デッドロックしたままでも
(そのスレッド自体は残り続けるが)テストプロセスが停止することはなく、
`timeout`経過後に`false`を返して制御を戻せる。

## 成果物イメージ
3種類の送金方式(素朴な実装/`scoped_lock`/タイムアウト付きリトライ)を比較する
デモCLI。

## ビルド方法

```sh
# 55_DeadlockAvoidance ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/DeadlockAvoidance
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/bank_account.h` | `BankAccount`。`std::timed_mutex`で保護された残高 | (下記各Transfer関数の土台) |
| `include/transfer.h`/`src/transfer.cpp` | `TransferNaive`/`TransferSafe`/`TransferWithTimeout`の3関数 | 下記参照 |
| `src/main.cpp` | 3方式を比較する統合デモ | 全体の統合デモ |

- `TransferNaive(from, to, amount)`: `from`→`to`の順に個別にロックする素朴な実装。
  ロック取得の間に意図的な小さいsleepを入れており、逆方向の同時送金で確実に
  デッドロックを再現できる。(デッドロックの再現)
- `TransferSafe(from, to, amount)`: `std::scoped_lock(from.GetMutex(), to.GetMutex())`
  で両方のロックを原子的に取得する。(std::scoped_lockによる回避)
- `TransferWithTimeout(from, to, amount, timeout)`: `try_lock_for`でタイムアウト
  付きにロックを試み、2つ目が取れなければ1つ目も手放してリトライする。
  (タイムアウト検知して諦める方式)
- ロック順序の統一による回避は、`TransferNaive`を常に「口座IDの小さい方→大きい方」
  等、決まった順序でだけ呼び出す運用ルールとして実現できる(コード上の追加実装は
  不要なため、本README上での言及のみ)。

## 動作確認

- `test/transfer_test.cpp`(7テスト):
  - 3関数それぞれの単一スレッドでの正しさ(残高の増減)を確認。
  - `TransferNaiveDeadlocksUnderOppositeDirectionContention`: A→B、B→Aの逆方向に
    `TransferNaive`を同時に呼ぶと`RunWithTimeout`が`false`(デッドロック)を
    返すことを確認(デッドロックの再現そのものが期待値)。
  - `TransferSafeDoesNotDeadlockUnderOppositeDirectionContention`: 同じ逆方向
    同時呼び出しを`TransferSafe`で行っても`RunWithTimeout`が`true`(時間内に完了)
    を返すことを確認。`TransferSafe`を`TransferNaive`と同じ個別ロックの実装に
    差し替えてテストを実行し、`false`(デッドロック)を返す(Red)ことを確認した
    上で、`std::scoped_lock`を使う実装に戻してGreen化した。
  - `TransferWithTimeoutDoesNotDeadlockUnderOppositeDirectionContention`: 同様に
    `TransferWithTimeout`でも`true`を返すことを確認。2つ目のロック取得に
    失敗した際に1つ目を手放さずそのままブロッキングロックしてしまう実装
    (デッドロックを確実に再現するための一時的なsleep付き)に差し替えてテストを
    実行し、`false`(デッドロック)を返す(Red)ことを確認した上で、手放して
    リトライする実装に戻してGreen化した。
  - `TransferSafePreservesTotalBalanceUnderConcurrentTransfers`: 8スレッドが
    `TransferSafe`で送金し合っても、2口座の残高合計が最初の合計と一致することを
    確認(ストレステスト)。
  - Red確認中、デッドロックする可能性のあるテスト(`TransferSafeDoesNot...`
    `TransferWithTimeoutDoesNot...`)を意図的に壊した際は、他の全テストを含む
    フルスイート実行だと後続の`TransferSafePreservesTotalBalance...`
    (`RunWithTimeout`未使用、素の`join()`)がデッドロックしてテストプロセス
    自体が無期限停止する事象が実際に発生した。該当テストのみ
    `--gtest_filter`で絞り込んで実行することでこれを回避しつつRedを確認した。
- `DeadlockAvoidanceTests.exe`実行でテスト7件全てパスすることを、タイムアウト
  監視付きで複数回(連続5回実行しても常にパスすること)確認。
- `DeadlockAvoidance.exe`実行で、`TransferNaive`はタイムアウト(デッドロック検知)、
  `TransferSafe`/`TransferWithTimeout`は完了することを確認した。終了コード0で
  終わることも確認した。
