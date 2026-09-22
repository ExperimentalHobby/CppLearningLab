#include "thread_safe_counter.h"

#include <gtest/gtest.h>

#include <thread>
#include <vector>

using namespace concurrency;

// mutexで排他制御されていれば、スレッド数×スレッドあたりのincrement回数が
// スケジューリングに関わらず必ず一致する(lost updateが起きない)。
TEST(ThreadSafeCounterTest, IncrementsCorrectlyAcrossManyThreads) {
    constexpr int kThreadCount = 8;
    constexpr int kIncrementsPerThread = 10000;

    ThreadSafeCounter counter;
    std::vector<std::thread> threads;
    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&counter, kIncrementsPerThread] {
            for (int j = 0; j < kIncrementsPerThread; ++j) {
                counter.Increment();
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(counter.Value(), static_cast<int64_t>(kThreadCount) * kIncrementsPerThread);
}
