# 09. マルチスレッド基礎

## 目的
`std::thread`/`std::mutex`/`std::condition_variable`による基本的な並行処理を学ぶ。
18番(マルチスレッドGUI)や31-38番(非同期通信)の土台になる知識。

## 学習ポイント
- スレッドの起動・待ち合わせ(`std::thread::join`)
- 共有データへの排他アクセス(`std::mutex`/`std::lock_guard`)
- スレッド間の同期・通知(`std::condition_variable`)
- データ競合(race condition)を実際に発生させ、mutexで解消する体験

## 採用ライブラリ/ツール
- 標準ライブラリのみ(`<thread>`/`<mutex>`/`<condition_variable>`/`<atomic>`)

## 成果物イメージ
排他制御の有無でカウンタの集計結果がどう変わるかを比較し、
`std::condition_variable`を使ったブロッキングキューでproducer-consumer
パターンを動かすデモCLI。

## ビルド方法

01〜08と同様にCMakeを使う(詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照)。

```sh
# 09_ConcurrencyBasics ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/ConcurrencyBasics
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/unsafe_counter.h`(ヘッダーオンリー) | 排他制御なしの`UnsafeCounter`。複数スレッドから`Increment()`するとlost update(更新の欠落)が起きうる | データ競合(race condition)の体験 |
| `include/thread_safe_counter.h`/`src/thread_safe_counter.cpp` | `std::mutex`+`std::lock_guard`で保護した`ThreadSafeCounter` | `std::mutex`による排他制御 |
| `include/blocking_queue.h`(ヘッダーオンリーテンプレート、`BlockingQueue<T>`) | `std::mutex`+`std::condition_variable`によるブロッキングキュー。`Pop()`はキューが空の間ブロックし、`Push()`されると起床する | `std::condition_variable`によるスレッド間の同期・通知、producer-consumerパターン |
| `src/main.cpp` | ①`UnsafeCounter`での競合実演 ②`ThreadSafeCounter`での正しい集計 ③`BlockingQueue`を使った複数producer/複数consumerのジョブ処理デモ | 全体の統合デモ |

`BlockingQueue<T>`はクラステンプレートのため、08番の`FixedStack`/`FixedQueue`と
同様にヘッダーオンリーで実装している(実際に使われた型でインスタンス化されて
初めてコードが実体化するため)。

## 動作確認

- `test/thread_safe_counter_test.cpp`: 8スレッド×10000回の`Increment()`後、
  合計が必ず期待値(80000)と一致することを確認。実装時に一度、意図的に
  `std::mutex`での保護を外した状態でテストを実行し、実際に更新が欠落して
  期待値と食い違う(Red)ことを確認した上で、`std::mutex`による保護を
  加えて常に一致する(Green)ことを確認した。
- `test/blocking_queue_test.cpp`: push→popの往復、`Size()`が空・Push後・
  Pop後の状態を正しく反映すること、`Pop()`がまだ何も`Push()`されていない
  間ブロックし、他スレッドが`Push()`した時点で起床すること、複数producer
  (4スレッド×2000件)×複数consumer(3スレッド)で送信した全8000件が
  重複・欠落なく届くことを確認。`Pop()`から`pop_front()`を意図的に外した
  状態でこのテストを実行し、同じ値を繰り返し受信して大半のアイテムが
  「届いていない」ことになる(Red)ことを確認した上で、正しい実装に戻して
  全件が一致する(Green)ことを確認した。`Pop()`がブロックしない実装や
  `Size()`が常に0を返す実装に一時的に差し替えた場合も、対応するテストが
  それぞれ確実に失敗する(Red)ことを確認済み。
- `ConcurrencyBasicsTests.exe`実行でテスト5件全てパスすることを複数回
  (連続実行しても常にパスすること)確認。
- `ConcurrencyBasics.exe`実行で、`UnsafeCounter`は期待値(800000)より
  少ない値になること(データ競合の実演。タイミング次第でまれに一致する
  こともある)、`ThreadSafeCounter`は常に期待値と一致すること、
  `BlockingQueue`のデモで送信した15件のjobが2つのconsumerスレッドで
  重複・欠落なく処理され終了コード0で終わることを確認した。
