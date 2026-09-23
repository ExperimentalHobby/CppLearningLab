# 51. std::atomic基礎

## 目的
`std::atomic`とmemory_orderの基礎を学ぶ。09番の`std::mutex`による排他制御とは異なる、
ロックを使わない同期の仕組みを理解する。

## 学習ポイント
- `std::atomic<T>`の基本操作(load/store/exchange)
- `compare_exchange_weak`/`strong`による原子的な条件付き更新
- memory_order(relaxed/acquire/release/seq_cst)の違いと使い分け
- 09番の`UnsafeCounter`/`ThreadSafeCounter`と同じカウンタを`std::atomic`だけで実装し、
  挙動を比較する

## 採用ライブラリ/ツール
- 標準ライブラリのみ(`<atomic>`)

## 成果物イメージ
`std::atomic`によるロックフリーカウンタ・最大値更新・自作スピンロックの3つを
比較するデモCLI。

## ビルド方法

```sh
# 51_AtomicOperations ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/AtomicOperations
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/atomic_counter.h`/`src/atomic_counter.cpp` | `std::atomic<int64_t>`の`fetch_add()`によるロックフリーカウンタ`AtomicCounter`。09番の`ThreadSafeCounter`(mutex版)と同条件で比較できる | load/store/exchange、09番との比較 |
| `include/atomic_max.h`/`src/atomic_max.cpp` | `UpdateMaxAtomic(std::atomic<int>&, int)`。`compare_exchange_weak`ループで「現在値より大きければ更新」を原子的に行う | compare_exchange_weak/strong |
| `include/spin_lock.h`/`src/spin_lock.cpp` | `std::atomic_flag`の`test_and_set`/`clear`による自作`SpinLock`。`lock()`はacquire、`unlock()`はreleaseを明示 | memory_order(acquire/release) |
| `src/main.cpp` | ①`AtomicCounter`の集計 ②`UpdateMaxAtomic`による最大値更新 ③`SpinLock`で保護したカウンタ、の3デモ | 全体の統合デモ |

## 動作確認

- `test/atomic_counter_test.cpp`: 8スレッド×100000回の`Increment()`後、合計が
  期待値(800000)と一致することを確認。`Increment()`を`load()`→加算→`store()`に
  意図的に分割した実装(fetch_add()を使わない)に差し替えてテストを実行し、
  実際にlost updateが起きて期待値と食い違う(Red、実測139万→13万台まで低下)ことを
  確認した上で、`fetch_add()`に戻して常に一致する(Green)ことを確認した。
- `test/atomic_max_test.cpp`:
  - `SingleThreadKeepsLargerValue`: 単一スレッドでの基本動作(小さい値では
    更新されない、大きい値では更新される)を確認。
  - `DoesNotRegressWhenTwoThreadsRaceSimultaneously`: 2スレッドを同時に開始させ、
    大きい値(800)を渡した側と小さい値(200)を渡した側が競合しても、最終的に
    大きい方(800)が残ることを確認。`load()`→比較→`store()`を分割した素朴な
    実装に、値が大きいほど短く小さいほど長くスリープする一時的な仕込みを入れて
    確実に競合を再現させ、大きい値が先にstoreされた後、古い値を読んでいた側の
    小さい値のstoreで上書きされてしまう(Red、200に巻き戻る)ことを実測で
    確認した上で、`compare_exchange_weak`ループに置き換えて解消した(Green)。
  - `ConvergesToActualMaximumUnderContention`: 16スレッド×3000回、乱数値で
    競合させても最終値が実際の最大値と一致することを確認(ストレステスト)。
- `test/spin_lock_test.cpp`: 8スレッド×100000回、`SpinLock`で保護した非atomicな
  共有intのインクリメントでlost updateが起きないことを確認。`lock()`/`unlock()`を
  実質no-op(排他が全く効かない実装)に差し替えてテストを実行し、実際に更新が
  欠落する(Red、実測80万→15万台まで低下)ことを確認した上で、
  `std::atomic_flag`による正しい実装に戻して常に一致する(Green)ことを確認した。
- `AtomicOperationsTests.exe`実行でテスト5件全てパスすることを複数回
  (連続5回実行しても常にパスすること)確認。
- `AtomicOperations.exe`実行で、3つのデモ全てが期待通りの値で一致し、
  終了コード0で終わることを確認した。
