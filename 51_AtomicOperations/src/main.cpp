// 51. std::atomic基礎
//
// std::atomicによるロックフリー同期を、3つのデモで確認する。
#include <windows.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#include "atomic_counter.h"
#include "atomic_max.h"
#include "spin_lock.h"

namespace {

constexpr int kThreadCount = 8;
constexpr int kIncrementsPerThread = 100000;

// デモ1: std::atomic<int64_t>のfetch_add()によるロックフリーカウンタ。
// 09番のThreadSafeCounter(std::mutex版)と同じ条件で必ず期待値と一致する。
void RunAtomicCounterDemo() {
    concurrency::AtomicCounter counter;
    std::vector<std::thread> threads;
    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&counter] {
            for (int j = 0; j < kIncrementsPerThread; ++j) {
                counter.Increment();
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    const int64_t expected = static_cast<int64_t>(kThreadCount) * kIncrementsPerThread;
    const int64_t actual = counter.Value();
    std::cout << "[AtomicCounter] 期待値=" << expected << " 実際の値=" << actual
              << (actual == expected ? " (一致。mutexを使わずロックフリーで排他制御を実現)" : " (不一致)")
              << "\n";
}

// デモ2: compare_exchange_weak()による原子的な最大値更新。複数スレッドが
// 乱数値で競合してもUpdateMaxAtomic()を通せば最終値は正しい最大値になる。
void RunAtomicMaxDemo() {
    constexpr int kThreadCountForMax = 8;
    constexpr int kUpdatesPerThread = 2000;

    std::atomic<int> maxValue{-1};
    int actualMax = -1;
    std::mutex actualMaxMutex;
    std::vector<std::thread> threads;
    for (int t = 0; t < kThreadCountForMax; ++t) {
        threads.emplace_back([&maxValue, &actualMax, &actualMaxMutex, t, kUpdatesPerThread] {
            std::mt19937 rng(static_cast<unsigned int>(t) + 1);
            std::uniform_int_distribution<int> dist(0, 1000000);
            for (int i = 0; i < kUpdatesPerThread; ++i) {
                const int candidate = dist(rng);
                concurrency::UpdateMaxAtomic(maxValue, candidate);
                std::lock_guard<std::mutex> lock(actualMaxMutex);
                if (candidate > actualMax) {
                    actualMax = candidate;
                }
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "[UpdateMaxAtomic] 実際の最大値=" << actualMax << " target=" << maxValue.load()
              << (actualMax == maxValue.load() ? " (一致)" : " (不一致)") << "\n";
}

// デモ3: std::atomic_flagで自作したSpinLockで非atomicな共有intを保護し、
// std::mutexと同様にlost updateが起きないことを確認する。
void RunSpinLockDemo() {
    concurrency::SpinLock lock;
    int sharedValue = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&lock, &sharedValue] {
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

    const int expected = kThreadCount * kIncrementsPerThread;
    std::cout << "[SpinLock] 期待値=" << expected << " 実際の値=" << sharedValue
              << (sharedValue == expected ? " (一致。atomic_flag+memory_orderで排他制御を実現)" : " (不一致)")
              << "\n";
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "=== デモ1: AtomicCounter(fetch_add)によるロックフリーカウンタ ===\n";
    RunAtomicCounterDemo();

    std::cout << "\n=== デモ2: UpdateMaxAtomic(compare_exchange_weak)による最大値更新 ===\n";
    RunAtomicMaxDemo();

    std::cout << "\n=== デモ3: SpinLock(atomic_flag)による排他制御 ===\n";
    RunSpinLockDemo();

    return 0;
}
