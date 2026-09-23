#include "thread_pool.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

using namespace async_ops;

// ワーカー数2のプールに、互いの開始を待ち合ってから完了する2つのタスクを
// 投入する。プールが本当に2スレッドを並行実行していなければ、片方が
// 完了するまでもう片方が開始できず、以下の「両方が開始するまで待つ」ロジックが
// タイムアウトしてfalseを返す。
//
// startedCountをポーリングする実装(1msスリープでスピンウェイト)だと、
// 高負荷なCI環境ではスケジューリング遅延で誤って失敗しうる一方、
// 単一ワーカーしか無い(=バグがある)場合は毎回タイムアウト分をまるごと
// 待たされてしまう(Copilotレビュー指摘)。condition_variableで
// 「値が変わったら即座に起床する」設計にすることで、成功時は
// ポーリング間隔による遅延なく即座に検出でき、失敗時も同じタイムアウトで
// 確実に打ち切れる。
TEST(ThreadPoolTest, ConstructsWithFixedWorkerCountAndRunsTasksConcurrently) {
    ThreadPool pool(2);
    std::mutex mutex;
    std::condition_variable cv;
    int startedCount = 0;

    auto task = [&] {
        {
            std::lock_guard<std::mutex> lock(mutex);
            ++startedCount;
        }
        cv.notify_all();

        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, std::chrono::seconds(1), [&] { return startedCount >= 2; });
    };

    std::future<bool> result1 = pool.Enqueue(task);
    std::future<bool> result2 = pool.Enqueue(task);

    EXPECT_TRUE(result1.get());
    EXPECT_TRUE(result2.get());
}

TEST(ThreadPoolTest, EnqueueReturnsFutureWithCorrectResult) {
    ThreadPool pool(2);

    std::future<int> result = pool.Enqueue([](int a, int b) { return a + b; }, 3, 4);

    EXPECT_EQ(result.get(), 7);
}

// ワーカー数より多いタスクを投入しても、全てのタスクが正しい結果を返すこと
// (使い回されるワーカーが正しくタスクを処理し続けること)を確認する。
TEST(ThreadPoolTest, MultipleTasksAllCompleteWithCorrectResults) {
    constexpr int kTaskCount = 50;
    ThreadPool pool(4);

    std::vector<std::future<int>> results;
    for (int i = 0; i < kTaskCount; ++i) {
        results.push_back(pool.Enqueue([](int x) { return x * x; }, i));
    }

    for (int i = 0; i < kTaskCount; ++i) {
        EXPECT_EQ(results[static_cast<size_t>(i)].get(), i * i);
    }
}

// プールを破棄する時点でキューに残っているタスクも、破棄が完了するまでに
// 全て実行され終わっていることを確認する。
TEST(ThreadPoolTest, DestructorWaitsForPendingTasksToComplete) {
    constexpr int kTaskCount = 20;
    std::atomic<int> completedCount{0};

    {
        ThreadPool pool(2);
        for (int i = 0; i < kTaskCount; ++i) {
            pool.Enqueue([&completedCount] {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                completedCount.fetch_add(1);
            });
        }
    }  // ここでデストラクタが呼ばれ、残タスクの完了を待ってから戻るはず

    EXPECT_EQ(completedCount.load(), kTaskCount);
}

TEST(ThreadPoolTest, EnqueueAfterShutdownThrows) {
    ThreadPool pool(2);
    pool.Shutdown();

    EXPECT_THROW(pool.Enqueue([] { return 0; }), std::runtime_error);
}

// numThreads==0だとワーカーが1つも起動されず、Enqueue()したタスクの
// futureが永久に完了しない(WorkerLoop()が誰も動かないため)。この事態を
// 未然に防ぐため、コンストラクタの時点で拒否することを確認する
// (Copilotレビュー指摘)。
TEST(ThreadPoolTest, ConstructingWithZeroWorkersThrows) {
    EXPECT_THROW(ThreadPool(0), std::invalid_argument);
}
