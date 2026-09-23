#include "atomic_max.h"

#include <gtest/gtest.h>

#include <atomic>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

using namespace concurrency;

// 2スレッドを同時に開始させ、大きい値のstoreが先に終わった後に小さい値
// のstoreが上書きしてしまわないことを確認する(TOCTOUの回帰テスト)。
// load()→比較・store()を分割した素朴な実装だと、大きい値(800)を
// storeした側より後に、既に古いcurrentを読んでいた側が小さい値(200)を
// storeしてしまい上書きされる欠陥を実際に確認した上で、
// compare_exchange_weakループに置き換えて解消した。
TEST(AtomicMaxTest, DoesNotRegressWhenTwoThreadsRaceSimultaneously) {
    std::atomic<int> target{-1};
    std::atomic<bool> start{false};

    std::thread big([&target, &start] {
        while (!start.load()) {
        }
        UpdateMaxAtomic(target, 800);
    });
    std::thread small([&target, &start] {
        while (!start.load()) {
        }
        UpdateMaxAtomic(target, 200);
    });
    start.store(true);
    big.join();
    small.join();

    EXPECT_EQ(target.load(), 800);
}

TEST(AtomicMaxTest, SingleThreadKeepsLargerValue) {
    std::atomic<int> target{5};

    UpdateMaxAtomic(target, 3);
    EXPECT_EQ(target.load(), 5);

    UpdateMaxAtomic(target, 10);
    EXPECT_EQ(target.load(), 10);
}

// 複数スレッドが乱数値でUpdateMaxAtomic()を競合させ、最終的にtargetが
// 「実際に渡された値の中の最大値」と必ず一致することを確認する。
// 「実際の最大値」は、テスト専用のstd::mutexで保護した変数(actualMax)に
// 別途正しく集計しておき、それとtargetを突き合わせる。
//
// 各スレッドが同じ値域を昇順で送るような設計だと、全スレッドが最後に
// 必ず同じ最大値を送ることになり、途中でTOCTOUによる巻き戻りが起きても
// 最終値だけを見ると偶然正しく収束してしまい検出できない。乱数値に
// することで「本来の最大値が確定した後に、別スレッドの巻き戻り書き込み
// (古いcurrentを読んだままの、本来の最大値未満の値によるstore)が
// target を上書きしてしまう」というTOCTOUの症状を最終値の不一致として
// 検出できるようにしている。
TEST(AtomicMaxTest, ConvergesToActualMaximumUnderContention) {
    constexpr int kThreadCount = 16;
    constexpr int kUpdatesPerThread = 3000;

    std::atomic<int> target{-1};
    int actualMax = -1;
    std::mutex actualMaxMutex;
    std::vector<std::thread> threads;
    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([&target, &actualMax, &actualMaxMutex, t, kUpdatesPerThread] {
            std::mt19937 rng(static_cast<unsigned int>(t) + 1);
            std::uniform_int_distribution<int> dist(0, 1000);
            for (int i = 0; i < kUpdatesPerThread; ++i) {
                const int candidate = dist(rng);
                UpdateMaxAtomic(target, candidate);
                std::lock_guard<std::mutex> lock(actualMaxMutex);
                if (candidate > actualMax) {
                    actualMax = candidate;
                }
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }

    EXPECT_EQ(target.load(), actualMax);
}
