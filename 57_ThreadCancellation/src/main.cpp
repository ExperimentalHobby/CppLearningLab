// 57. スレッドのキャンセル・タイムアウト処理
//
// std::atomic<bool>のキャンセルフラグ+std::condition_variableによる
// 協調的キャンセルを、3つのデモで確認する。
#include <windows.h>

#include <atomic>
#include <chrono>
#include <iostream>

#include "cancellable_task.h"
#include "timeout_runner.h"

namespace {

// デモ1: workUnitが自発的に終わり、キャンセルなしで正常終了する。
void RunNormalCompletionDemo() {
    std::atomic<int> callCount{0};
    concurrency::CancellableTask task([&callCount] { return callCount.fetch_add(1) < 4; },
                                       std::chrono::milliseconds(10));

    task.Start();
    task.WaitForCompletion(std::chrono::seconds(2));
    task.Join();

    std::cout << "[NormalCompletion] 呼び出し回数=" << callCount.load()
              << " キャンセルされたか=" << (task.WasCancelled() ? "true" : "false") << "\n";
}

// デモ2: 自発的には終わらないタスクを、起動直後にRequestCancel()で
// 即座に停止させる(pollIntervalの満了を待たない)。
void RunPromptCancelDemo() {
    concurrency::CancellableTask task([] { return true; }, std::chrono::seconds(5));

    const auto start = std::chrono::steady_clock::now();
    task.Start();
    task.RequestCancel();
    task.WaitForCompletion(std::chrono::seconds(2));
    const auto elapsedMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    task.Join();

    std::cout << "[PromptCancel] 停止までの時間=" << elapsedMs << "ms(pollInterval=5000msだが即座に停止)\n";
}

// デモ3: タイムアウト付きでタスクを実行し、時間内に終わらなければ
// 自動的にキャンセルする。
void RunTimeoutDemo() {
    concurrency::CancellableTask task([] { return true; }, std::chrono::milliseconds(10));

    const bool completedInTime = concurrency::RunWithTimeout(task, std::chrono::milliseconds(200));

    std::cout << "[Timeout] 時間内に完了=" << (completedInTime ? "true" : "false")
              << " キャンセルされたか=" << (task.WasCancelled() ? "true" : "false") << "\n";
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "=== デモ1: 正常終了(キャンセルなし) ===\n";
    RunNormalCompletionDemo();

    std::cout << "\n=== デモ2: 即時キャンセル(condition_variableによる即応) ===\n";
    RunPromptCancelDemo();

    std::cout << "\n=== デモ3: タイムアウトによる自動キャンセル ===\n";
    RunTimeoutDemo();

    return 0;
}
