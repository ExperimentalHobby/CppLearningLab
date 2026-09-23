#include "cancellable_task.h"
#include "timeout_runner.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

using namespace concurrency;

TEST(CancellableTaskTest, TaskCompletesNormallyWhenWorkUnitFinishes) {
    std::atomic<int> callCount{0};
    CancellableTask task([&callCount] { return callCount.fetch_add(1) < 2; },
                          std::chrono::milliseconds(10));

    task.Start();
    ASSERT_TRUE(task.WaitForCompletion(std::chrono::seconds(2)));
    task.Join();

    EXPECT_FALSE(task.WasCancelled());
    EXPECT_EQ(callCount.load(), 3);
}

// pollIntervalを長く(5秒)設定し、workUnitは常にtrueを返す(自発的には
// 終わらない)タスクを用意する。workUnitが1回呼ばれたことを示すstartedフラグを
// 確認してから(=ワーカースレッドがwait_for(5秒)による待機に入ったタイミング
// 付近で)RequestCancel()を呼ぶ。condition_variableのnotify_allが正しく
// 機能していれば、pollIntervalの満了を待たずに短時間で完了を検知できるはず。
TEST(CancellableTaskTest, RequestCancelStopsTaskPromptlyEvenDuringWait) {
    std::atomic<bool> started{false};
    CancellableTask task(
        [&started] {
            started.store(true);
            return true;
        },
        std::chrono::seconds(5));

    task.Start();
    while (!started.load()) {
    }
    // workUnit()から戻った直後、ワーカースレッドが実際にwait_for()へ入る
    // までの短い猶予を与える。
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const auto start = std::chrono::steady_clock::now();
    task.RequestCancel();
    const bool completed = task.WaitForCompletion(std::chrono::seconds(2));
    const auto elapsed = std::chrono::steady_clock::now() - start;
    task.Join();

    EXPECT_TRUE(completed);
    EXPECT_TRUE(task.WasCancelled());
    EXPECT_LT(elapsed, std::chrono::seconds(1))
        << "RequestCancel()がnotifyしていないと、pollInterval(5秒)満了まで応答しない";
}

TEST(CancellableTaskTest, RunWithTimeoutCancelsTaskThatDoesNotFinishInTime) {
    CancellableTask task([] { return true; }, std::chrono::milliseconds(10));

    const bool completedInTime = RunWithTimeout(task, std::chrono::milliseconds(100));

    EXPECT_FALSE(completedInTime);
    EXPECT_TRUE(task.WasCancelled()) << "タイムアウト時にRequestCancel()が呼ばれていない";
}

TEST(CancellableTaskTest, RunWithTimeoutReturnsTrueForTaskThatFinishesInTime) {
    std::atomic<int> callCount{0};
    CancellableTask task([&callCount] { return callCount.fetch_add(1) < 2; },
                          std::chrono::milliseconds(10));

    const bool completedInTime = RunWithTimeout(task, std::chrono::seconds(2));

    EXPECT_TRUE(completedInTime);
    EXPECT_FALSE(task.WasCancelled());
}
