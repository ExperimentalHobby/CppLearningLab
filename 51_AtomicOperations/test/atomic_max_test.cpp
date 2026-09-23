#include "atomic_max.h"

#include <gtest/gtest.h>

#include <atomic>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

using namespace concurrency;

// afterLoadHookを使い、以下の順序を決定的に強制した上で、大きい値(800)を
// 渡した側と小さい値(200)を渡した側を競合させ、最終的に大きい方(800)が
// 残ることを確認する(TOCTOUの回帰テスト)。
//   1. big/smallの両方がload()を完了する(=両方とも古い値-1を読む)
//   2. bigが先にcompare/storeへ進み、800をstoreし終える
//   3. bigのstore完了を確認してから、smallが(手順1で読んだ古いcurrent=-1の
//      ままで)compare/storeへ進む
//
// 単に開始タイミングを揃えるだけのstartフラグでは、両方がload()を完了する
// 前にどちらかがstoreまで進んでしまう順序も、bigとsmallのstoreの前後関係も
// 制御できておらず、壊れたload→比較→store実装でも偶然「big側のstoreが
// small側のstoreより後」の順序になればテストが通ってしまい、TOCTOU回帰を
// 確実には検出できなかった(Copilotレビュー指摘)。上記の2段階の同期に
// より、bigのstoreが完了した後にsmallが古いcurrentのままstoreを試みる、
// という欠陥が確実に顕在化する順序を100%決定的に再現する。
//
// load()→比較・store()を分割した素朴な実装だと、この決定的な順序下で
// 大きい値(800)をstoreした後に、古いcurrent(-1)を読んでいたsmallが
// 小さい値(200)を無条件にstoreしてしまい上書きされる欠陥を実際に確認した
// 上で、compare_exchange_weakループに置き換えて解消した(小さい値を読んで
// いた側のCASは、targetが既に800に変わっているため失敗し、currentが800に
// 更新された上で200>800がfalseとなり、上書きされずに済む)。
TEST(AtomicMaxTest, DoesNotRegressWhenTwoThreadsRaceSimultaneously) {
    std::atomic<int> target{-1};
    std::atomic<int> loadedCount{0};
    std::atomic<bool> bigStored{false};

    const auto bigHook = [&loadedCount] {
        loadedCount.fetch_add(1);
        while (loadedCount.load() < 2) {
        }
        // bigはここから即座にcompare/storeへ進む(待たない)。
    };
    const auto smallHook = [&loadedCount, &bigStored] {
        loadedCount.fetch_add(1);
        while (loadedCount.load() < 2) {
        }
        // bigのstoreが完了するまで待ってから、(手順1で読んだ)古い
        // currentのままcompare/storeへ進む。
        while (!bigStored.load()) {
        }
    };

    std::thread big([&target, &bigHook, &bigStored] {
        UpdateMaxAtomic(target, 800, bigHook);
        bigStored.store(true);
    });
    std::thread small([&target, &smallHook] { UpdateMaxAtomic(target, 200, smallHook); });
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
