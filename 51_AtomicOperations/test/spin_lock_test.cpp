#include "spin_lock.h"

#include <gtest/gtest.h>

#include <thread>
#include <vector>

using namespace concurrency;

// SpinLockで保護した非atomicな共有intは、std::mutex+lock_guardと同様に
// lost updateが起きないことを確認する(mutual exclusionの実演)。
TEST(SpinLockTest, ProtectsSharedIntFromLostUpdatesAcrossManyThreads) {
    constexpr int kThreadCount = 8;
    constexpr int kIncrementsPerThread = 100000;

    SpinLock lock;
    int sharedValue = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&lock, &sharedValue, kIncrementsPerThread] {
            for (int j = 0; j < kIncrementsPerThread; ++j) {
                lock.lock();
                ++sharedValue;
                lock.unlock();
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(sharedValue, kThreadCount * kIncrementsPerThread);
}
