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

- `test/thread_pool_test.cpp`(6テスト):
  - `ConstructsWithFixedWorkerCountAndRunsTasksConcurrently`: ワーカー数2のプールに、
    互いの開始を待ち合ってから完了する2つのタスクを投入し、両方が時間内に完了する
    (=本当に2スレッド並行実行されている)ことを確認。開始通知は`condition_variable`
    で行っており、成功時はポーリング間隔による遅延なく即座に検出でき、失敗時も
    タイムアウト(1秒)で確実に打ち切れる(スリープでのポーリングは、高負荷なCI
    環境での誤検出や、失敗時に毎回タイムアウト分をまるごと待たされる問題が
    あったため、Copilotレビュー指摘を受けて書き換えた)。コンストラクタで
    `numThreads - 1`個のワーカーしか起動しない実装に差し替えてテストを実行し、
    2つ目のタスクが1つ目の完了を待てず(ワーカーが1つしか無いため)1秒のタイム
    アウトで`false`を返す(Red)ことを確認した上で、`numThreads`個に戻して
    Green化した。
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
  - `ConstructingWithZeroWorkersThrows`: `numThreads==0`だとワーカーが1つも
    起動されず、投入したタスクの`future`が誰にも処理されず永久に完了しない
    (`Shutdown()`で待っても終わらない)ため、コンストラクタの時点で
    `std::invalid_argument`を送出して拒否することを確認(Copilotレビュー指摘、
    ガードを実装する前は例外が送出されない(Red)ことを確認した上で、
    `numThreads==0`を拒否するガードを追加してGreen化した)。
- コンストラクタの例外安全性(Copilotレビュー指摘): 一部のワーカーだけ起動済みの
  状態で`std::thread`の構築が失敗すると(OSのスレッド/リソース上限到達等)、
  このオブジェクト自体の構築が失敗するため`~ThreadPool()`は呼ばれず、
  `workers_`(既にjoinable()な`std::thread`を含む)だけがそのまま破棄され、
  `std::thread`のデストラクタが`std::terminate()`を呼んでしまう欠陥があった。
  コンストラクタ内をtry/catchで囲み、失敗時は起動済みのワーカーを明示的に
  停止・joinしてから例外を再送出するよう修正した。OSのスレッド/リソース枯渇を
  意図的かつ確実に発生させることは実行環境に依存し自動テストでの再現が
  難しいため、この修正はコードレビュー(例外安全性の推論)によって正しさを
  確認しており、自動テストの対象にはしていない。
- `/EHsc`の明示(Copilotレビュー指摘): `ThreadPool`が`std::invalid_argument`/
  `std::runtime_error`を送出するようになったのに対し、`CMakeLists.txt`では
  MSVC向けに`/utf-8`のみ指定しており`/EHsc`(C++例外の巻き戻しセマンティクス)が
  明示されていなかったため追加した。
- `<memory>`/`<utility>`の直接インクルード(Copilotレビュー指摘): `thread_pool.h`が
  `std::make_shared`/`std::forward`を使っているにもかかわらず、`<future>`/
  `<functional>`経由の間接インクルードに依存していたため、ヘッダー単体で
  自己完結するよう明示的に追加した。
- `ThreadPoolTests.exe`実行でテスト6件全てパスすることを複数回
  (連続5回実行しても常にパスすること)確認。
- `ThreadPool.exe`実行で、4ワーカーに投入した10件のタスク全てが正しい結果
  (0^2〜9^2)で完了し、終了コード0で終わることを確認した。
