// 09. マルチスレッド基礎
//
// std::thread/std::mutex/std::condition_variableによる並行処理の基本を、
// 3つのデモで確認する。
#include <windows.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "blocking_queue.h"
#include "thread_safe_counter.h"
#include "unsafe_counter.h"

namespace {

constexpr int kThreadCount = 8;
constexpr int kIncrementsPerThread = 100000;

// デモ1: 排他制御なしのUnsafeCounterで複数スレッドから同時にIncrement()し、
// データ競合(lost update)によって最終値が期待値よりも小さくなりうることを
// 見せる。タイミング次第でたまたま一致することもあるため、必ず不一致に
// なるわけではない点に注意。
void RunUnsafeCounterDemo() {
    concurrency::UnsafeCounter counter;
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
    std::cout << "[UnsafeCounter] 期待値=" << expected << " 実際の値=" << counter.Value();
    if (counter.Value() != expected) {
        std::cout << " (データ競合により更新が失われました)";
    } else {
        std::cout << " (たまたま一致しましたが、排他制御が無いため保証はありません)";
    }
    std::cout << "\n";
}

// デモ2: std::mutexで保護されたThreadSafeCounterでは、同じ条件でも
// 必ず期待値と一致する。
void RunThreadSafeCounterDemo() {
    concurrency::ThreadSafeCounter counter;
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
    std::cout << "[ThreadSafeCounter] 期待値=" << expected << " 実際の値=" << counter.Value()
              << (counter.Value() == expected ? " (一致)" : " (不一致)") << "\n";
}

// デモ3: BlockingQueue<T>によるproducer-consumerパターン。複数の
// producerスレッドがジョブ番号をPush()し、複数のconsumerスレッドが
// Pop()して処理する。Pop()はキューが空の間ブロックし、Push()されると
// std::condition_variableで起床する。
void RunBlockingQueueDemo() {
    constexpr int kProducerCount = 3;
    constexpr int kJobsPerProducer = 5;
    constexpr int kConsumerCount = 2;
    constexpr int kTotalJobs = kProducerCount * kJobsPerProducer;

    concurrency::BlockingQueue<int> jobs;
    std::vector<std::thread> producers;
    for (int p = 0; p < kProducerCount; ++p) {
        producers.emplace_back([&jobs, p, kJobsPerProducer] {
            for (int i = 0; i < kJobsPerProducer; ++i) {
                jobs.Push(p * kJobsPerProducer + i);
            }
        });
    }

    // 「Pop()する前にthis回のジョブ枠を予約できたか」をfetch_add()の戻り値
    // (加算前の値)で判定する。processedCount.Value() < kTotalJobsのような
    // 判定と実際のPop()の間に別スレッドが割り込む余地があると、既に全ジョブ
    // が払い出された後にPop()を呼んでしまい、以降Push()されないキューに
    // 対してブロックしたまま戻らなくなる(デッドロック)。fetch_add()は
    // 判定と予約を1つの原子操作で行うため、そのような競合が起きない。
    std::atomic<int> reservedCount{0};
    concurrency::ThreadSafeCounter processedCount;
    // std::coutへの<<の連鎖は1つの式全体としては atomic ではないため、
    // 複数スレッドから同時に出力すると行が混ざって文字化けのように見える。
    // 出力用に別途mutexで保護する(Push/Popの正しさ自体とは無関係な、
    // デモの見た目のための同期)。
    std::mutex coutMutex;
    std::vector<std::thread> consumers;
    for (int c = 0; c < kConsumerCount; ++c) {
        consumers.emplace_back([&jobs, &processedCount, &reservedCount, &coutMutex, c, kTotalJobs] {
            while (reservedCount.fetch_add(1) < kTotalJobs) {
                const int job = jobs.Pop();
                processedCount.Increment();
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "[consumer" << c << "] job " << job << " を処理しました\n";
            }
        });
    }

    for (auto& t : producers) {
        t.join();
    }
    for (auto& t : consumers) {
        t.join();
    }
    std::cout << "[BlockingQueue] 合計" << processedCount.Value() << "件のjobを処理しました\n";
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "=== デモ1: 排他制御なし(UnsafeCounter)でのデータ競合 ===\n";
    RunUnsafeCounterDemo();

    std::cout << "\n=== デモ2: std::mutexで保護したThreadSafeCounter ===\n";
    RunThreadSafeCounterDemo();

    std::cout << "\n=== デモ3: BlockingQueueによるproducer-consumer ===\n";
    RunBlockingQueueDemo();

    return 0;
}
