#include "atomic_counter.h"

#include <gtest/gtest.h>

#include <thread>
#include <vector>

using namespace concurrency;

// std::atomicで排他制御されていれば、mutex版のThreadSafeCounter(09番)と
// 同様に、スレッド数×スレッドあたりのincrement回数が必ず一致する。
TEST(AtomicCounterTest, IncrementsCorrectlyAcrossManyThreads) {
    constexpr int kThreadCount = 8;
    constexpr int kIncrementsPerThread = 100000;

    AtomicCounter counter;
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
