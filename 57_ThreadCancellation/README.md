# 57. スレッドのキャンセル・タイムアウト処理

## 目的
実行中のスレッドを安全に途中終了させる協調的キャンセルの仕組みを学ぶ。本
リポジトリはC++17を基準としているため、C++20の`std::jthread`/`std::stop_token`
は使わず、`std::atomic<bool>`のキャンセルフラグ+`std::condition_variable`で
自前実装する。

## 学習ポイント
- 強制終了(`std::thread::detach`後の放置やOSレベルの強制終了)ではなく、スレッド
  自身が定期的にキャンセル要求を確認して自発的に終了する協調的キャンセルの設計
- キャンセルフラグ+`condition_variable::wait_for`の組み合わせで、待機中でも
  速やかにキャンセルに応答できるようにする
- 長時間実行中のタスクに対してタイムアウトを設定し、時間内に終わらなければ
  キャンセルする仕組み

53番のスレッドプールへのキャンセル機能統合は、Issue本文で「余裕があれば」の
発展例として挙げられているが、本課題のスコープでは実装していない
(53番の`ThreadPool`自体への変更が必要になり、別課題として切り出すべき規模の
ため)。

## 採用ライブラリ/ツール
- 標準ライブラリのみ(`<atomic>`/`<condition_variable>`/`<thread>`)

## 成果物イメージ
正常終了・即時キャンセル・タイムアウトによる自動キャンセルの3パターンを
確認するデモCLI。

## ビルド方法

```sh
# 57_ThreadCancellation ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/ThreadCancellation
```

## 構成と学習ポイントとの対応

| ファイル | 内容 | 学習ポイント |
|---|---|---|
| `include/cancellable_task.h`/`src/cancellable_task.cpp` | `CancellableTask`。`workUnit`を繰り返し呼び出すワーカースレッドを保持し、`RequestCancel()`/`WaitForCompletion()`を提供 | 協調的キャンセルの設計、condition_variableによる即応 |
| `include/timeout_runner.h`/`src/timeout_runner.cpp` | `RunWithTimeout(task, timeout)`。時間内に終わらなければ`RequestCancel()`する | タイムアウトによる自動キャンセル |
| `src/main.cpp` | ①正常終了 ②即時キャンセル ③タイムアウトキャンセル、の3デモ | 全体の統合デモ |

`CancellableTask::Run()`はループの先頭で`cancelRequested_`を確認し、`workUnit`が
`false`を返すか、キャンセルが要求されるまで動作を続ける。イテレーションの間は
`cv_.wait_for(pollInterval)`で待機するが、`RequestCancel()`が`notify_all()`する
ため、待機中でも`pollInterval`の満了を待たずに即座に起床してキャンセル要求に
応答できる。

## 動作確認

- `test/cancellable_task_test.cpp`(4テスト):
  - `TaskCompletesNormallyWhenWorkUnitFinishes`: `workUnit`が自発的に`false`を
    返すと、キャンセルなしで正常終了することを確認。
  - `RequestCancelStopsTaskPromptlyEvenDuringWait`: `pollInterval`を5秒に設定した
    タスクが実際に`wait_for`による待機に入ったタイミングで`RequestCancel()`を
    呼び、`pollInterval`よりずっと短い時間(1秒未満)で完了を検知できることを
    確認。`RequestCancel()`が`notify_all()`を呼ばない実装に差し替えてテストを
    実行し、5秒の`pollInterval`満了まで応答できない(Red、テストが5秒かけて
    失敗)ことを確認した上で、`notify_all()`を呼ぶ実装に戻してGreen化した。
  - `RunWithTimeoutCancelsTaskThatDoesNotFinishInTime`: 自発的に終わらない
    タスクを短いタイムアウトで`RunWithTimeout`に渡すと、`false`を返し、かつ
    タスク自体が`RequestCancel()`されて`WasCancelled()==true`になることを確認。
    `RunWithTimeout`がタイムアウト時に`RequestCancel()`を呼ばずただ`false`を
    返すだけの実装に差し替えてテストを実行し、`WasCancelled()`が`false`のまま
    (Red、タスクを放置してしまっている)ことを確認した上で、`RequestCancel()`を
    呼ぶ実装に戻してGreen化した。
  - `RunWithTimeoutReturnsTrueForTaskThatFinishesInTime`: 短時間で終わるタスクを
    十分なタイムアウトで実行すると`true`を返すことを確認。
- `ThreadCancellationTests.exe`実行でテスト4件全てパスすることを、タイムアウト
  監視付きで複数回(連続5回実行しても常にパスすること)確認。
- `ThreadCancellation.exe`実行で、①正常終了(呼び出し回数どおり)、②即時
  キャンセル(pollInterval=5000msだがほぼ0msで停止)、③タイムアウトによる
  自動キャンセル(時間内に完了=false、キャンセルされたか=true)の全てが
  期待通りであり、終了コード0で終わることを確認した。
