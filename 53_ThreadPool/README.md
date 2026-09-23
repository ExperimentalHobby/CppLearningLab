# 53. スレッドプール

## 目的
固定数のワーカースレッドとタスクキューによるスレッドプールを実装する。09番の
`BlockingQueue<T>`をタスクキューとして応用し、52番の`std::future`で実行結果を
呼び出し元に返す、複数の並行処理技法を組み合わせた集大成的な課題。

## 学習ポイント
- 起動時に固定数のワーカースレッドを立ち上げ、以降は使い回す設計
- 任意の関数(タスク)をキューに投入し、空いているワーカースレッドが順次実行する仕組み
- `std::packaged_task`+`std::future`で、投入したタスクの戻り値を呼び出し元で受け取れる
  ようにする
- プールの終了処理(残タスクの完了待ち、ワーカースレッドの合流)

## 採用ライブラリ/ツール
- 標準ライブラリのみ(`<thread>`/`<mutex>`/`<condition_variable>`/`<future>`)

## 注意点: `namespace concurrency`との命名衝突

52番と同様、`<future>`をインクルードするとMSVCのConcurrency Runtime(PPL)の
ヘッダー`pplwin.h`が間接的に読み込まれ、そこで宣言されている`namespace concurrency`と
衝突する(`C2757`)ため、本課題でも`namespace async_ops`を使っている。

## 成果物イメージ
`ThreadPool`に複数のタスクを投入し、結果を`std::future`で受け取るデモCLI。

## ビルド方法

```sh
# 53_ThreadPool ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/ThreadPool
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/thread_pool.h`/`src/thread_pool.cpp` | `ThreadPool`。コンストラクタで固定数のワーカースレッドを起動し、`Enqueue()`でタスクを投入、`Shutdown()`/デストラクタで終了処理を行う | 全学習ポイント |
| `src/main.cpp` | プールに10個のタスクを投入し、結果をfutureで受け取るデモ | 全体の統合デモ |

`ThreadPool`内部のタスクキューは、09番の`BlockingQueue<T>`とは別に専用実装している。
`BlockingQueue<T>`は「stopを通知して待機を打ち切る」機能を持たず、終了処理(残タスクの
完了待ち)が要件にあるこのクラスには適さないため。

## 動作確認

- `test/thread_pool_test.cpp`(5テスト):
  - `ConstructsWithFixedWorkerCountAndRunsTasksConcurrently`: ワーカー数2のプールに、
    互いの開始を待ち合ってから完了する2つのタスクを投入し、両方が時間内に完了する
    (=本当に2スレッド並行実行されている)ことを確認。コンストラクタで
    `numThreads - 1`個のワーカーしか起動しない実装に差し替えてテストを実行し、
    2つ目のタスクが1つ目の完了を待てず(ワーカーが1つしか無いため)タイムアウトで
    `false`を返す(Red)ことを確認した上で、`numThreads`個に戻してGreen化した。
  - `EnqueueReturnsFutureWithCorrectResult`/`MultipleTasksAllCompleteWithCorrectResults`:
    引数付きタスクの実行結果が正しく`future`で受け取れることを確認。
    `Enqueue()`内部でキューに積むラムダが`packaged_task`を実際には呼び出さない
    実装に差し替えてテストを実行し、`future.get()`が`std::future_error`
    (broken promise)を送出する(Red)ことを確認した上で、`(*task)()`を正しく
    呼び出す実装に戻してGreen化した。
  - `DestructorWaitsForPendingTasksToComplete`: 複数タスクを投入した直後にプールを
    破棄し、破棄が完了した時点で全タスクの完了カウンタが投入数と一致することを
    確認。ワーカーループの終了条件を「stopフラグが立ったら残タスクの有無を見ずに
    即座に終了」する実装に差し替えてテストを実行し、キューに残っていたタスクが
    一つも実行されないままプールが破棄される(Red、カウンタが0のまま)ことを
    確認した上で、「stopが立っていてもキューが空になるまで処理を続ける」実装に
    戻してGreen化した。
  - `EnqueueAfterShutdownThrows`: `Shutdown()`後の`Enqueue()`が例外を送出することを
    確認。
- `ThreadPoolTests.exe`実行でテスト5件全てパスすることを複数回
  (連続5回実行しても常にパスすること)確認。
- `ThreadPool.exe`実行で、4ワーカーに投入した10件のタスク全てが正しい結果
  (0^2〜9^2)で完了し、終了コード0で終わることを確認した。
