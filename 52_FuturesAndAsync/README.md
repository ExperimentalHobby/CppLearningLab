# 52. std::future/std::asyncによる非同期処理

## 目的
`std::future`/`std::async`/`std::packaged_task`による非同期タスクの実行と結果取得を学ぶ。
09番の`BlockingQueue`による手動のproducer-consumerとは異なる、標準ライブラリが提供する
高レベルな非同期処理の抽象化を理解する。

## 学習ポイント
- `std::async`によるタスクの非同期実行と、`std::future`での結果取得(get/wait)
- `std::launch::async`/`deferred`の違い
- `std::packaged_task`による、実行するタスクと結果の受け取りの分離
- 例外を投げるタスクを`std::async`で実行し、`std::future::get()`側で例外が
  再送出されることの確認

## 採用ライブラリ/ツール
- 標準ライブラリのみ(`<future>`/`<thread>`)

## 注意点: `namespace concurrency`との命名衝突

09番/51番では`namespace concurrency`を使っているが、本課題では`namespace async_ops`を
使っている。これは`<future>`をインクルードすると、MSVCの実装上、Windows Concurrency
Runtime(PPL)のヘッダー`pplwin.h`が間接的に読み込まれ、そこで宣言されている
`namespace concurrency`(C++標準の並行処理とは無関係な、PPL独自の名前空間)と衝突して
`C2757`コンパイルエラーになるため。`windows.h`のグローバル関数・マクロとの衝突
(ルート[README.md](../README.md)の「よくある落とし穴」参照)と同種の、MSVC環境固有の
名前衝突である。

## 成果物イメージ
`std::async`による非同期計算・launch policyの違い・例外伝播・`std::packaged_task`の
4つを確認するデモCLI。

## ビルド方法

```sh
# 52_FuturesAndAsync ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/FuturesAndAsync
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/async_tasks.h`/`src/async_tasks.cpp` | `ComputeSquareAsync`/`GetExecutionThreadId`/`ThrowingTaskAsync`/`RunAddViaPackagedTask`の4関数 | 下記参照 |
| `src/main.cpp` | 上記4関数を使った統合デモ | 全体の統合デモ |

- `ComputeSquareAsync(int x)`: `std::async(std::launch::async, ...)`でxの平方を計算し、
  `std::future<int>`で結果を受け取る。(std::asyncによる非同期実行、future::get)
- `GetExecutionThreadId(std::launch policy)`: 指定したlaunch policyでタスクを実行し、
  そのタスクが実行されたスレッドのIDを返す。(launch::async/deferredの違い)
- `ThrowingTaskAsync()`: `std::async`で実行した先で例外を投げ、`future::get()`側で
  再送出されることを示す。(例外の再送出)
- `RunAddViaPackagedTask(int a, int b)`: `std::packaged_task`で「実行するタスク」と
  「結果の受け取り(future)」を分離する。(packaged_taskによる分離)

## 動作確認

- `test/async_tasks_test.cpp`(5テスト):
  - `ComputeSquareAsyncReturnsCorrectValue`: `x*x`ではなく`x+x`を返す誤った実装に
    差し替えて不一致を確認(Red)した上で、`x*x`に戻して一致を確認(Green)した。
  - `DeferredPolicyRunsOnCallingThreadWhenGetIsCalled`/`AsyncPolicyRunsOnDifferentThread`:
    渡された`policy`引数を無視して常に`std::launch::async`で実行する誤った実装に
    差し替え、`deferred`指定時のテストが「実行スレッドが呼び出し元と一致するはず」を
    満たせず失敗する(Red)ことを確認した上で、`policy`をそのまま`std::async`に渡す
    実装に戻して両テストが通る(Green)ことを確認した。
  - `ThrowingTaskPropagatesExceptionThroughFutureGet`: 例外を投げず`0`を返すだけの
    誤った実装に差し替えて`EXPECT_THROW`が失敗する(Red)ことを確認した上で、
    `std::runtime_error`を投げる実装に戻してGreenを確認した。
  - `PackagedTaskReturnsCorrectSum`: `a+b`ではなく`a-b`を計算する誤った実装に
    差し替えて不一致を確認(Red)した上で、`a+b`に戻して一致を確認(Green)した。
- `FuturesAndAsyncTests.exe`実行でテスト5件全てパスすることを複数回
  (連続5回実行しても常にパスすること)確認。
- `FuturesAndAsync.exe`実行で、4つのデモ全てが期待通りの結果(平方の計算結果、
  deferred/asyncでの実行スレッドIDの一致/不一致、例外メッセージの捕捉、
  packaged_taskでの加算結果)になり、終了コード0で終わることを確認した。
