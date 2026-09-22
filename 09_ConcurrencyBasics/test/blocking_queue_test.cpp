#include "blocking_queue.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

using namespace concurrency;

TEST(BlockingQueueTest, PushThenPopReturnsSameValue) {
    BlockingQueue<int> queue;

    queue.Push(42);

    EXPECT_EQ(queue.Pop(), 42);
}

// Pop()はキューが空の間ブロックし、別スレッドがPush()した時点で起床して
// 値を返すことを確認する。producer側をsleepさせてからPush()するだけでは、
// スケジューリング次第でPush()がPop()より先に実行されてしまい、
// 「Pop()が即座に返っても(実はブロックしていなくても)テストが通る」
// 可能性がある。そこでPop()を呼ぶ側を別スレッドにし、①一定時間待っても
// そのスレッドが完了していない(=ブロックしている)ことを先に確認してから
// ②Push()し、③直後に完了することを確認する、という順序で検証する。
TEST(BlockingQueueTest, PopBlocksUntilItemIsPushed) {
    BlockingQueue<int> queue;
    std::atomic<bool> started{false};
    std::atomic<bool> popped{false};
    int result = 0;

    std::thread consumer([&] {
        started = true;
        result = queue.Pop();
        popped = true;
    });

    // consumerスレッドがまだ実行開始すらしていない段階で「ブロックして
    // いない(=popped==false)」と判定してしまうと、Pop()が実際には
    // ブロックしない実装でも「単にスレッドがまだ動いていないだけ」で
    // テストが偽陽性で通ってしまう。startedがtrueになる(=consumerが
    // Pop()を呼び出した)まで待ってから、ブロックしているかどうかを判定する。
    while (!started.load()) {
        std::this_thread::yield();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(popped.load()) << "Push()前にPop()が返ってしまった(ブロックしていない)";

    queue.Push(7);
    consumer.join();

    EXPECT_TRUE(popped.load());
    EXPECT_EQ(result, 7);
}

// 複数producer×複数consumerで、送信した全アイテムが重複・欠落なく
// 届くことを確認する(スレッドセーフ性の核心)。
TEST(BlockingQueueTest, DeliversAllItemsExactlyOnceAcrossMultipleProducersAndConsumers) {
    constexpr int kProducerCount = 4;
    constexpr int kItemsPerProducer = 2000;
    constexpr int kConsumerCount = 3;
    constexpr int kTotalItems = kProducerCount * kItemsPerProducer;

    BlockingQueue<int> queue;
    std::vector<std::thread> producers;
    for (int p = 0; p < kProducerCount; ++p) {
        producers.emplace_back([&queue, p, kItemsPerProducer] {
            for (int i = 0; i < kItemsPerProducer; ++i) {
                queue.Push(p * kItemsPerProducer + i);
            }
        });
    }

    std::mutex resultMutex;
    std::set<int> received;
    std::atomic<int> poppedCount{0};
    std::vector<std::thread> consumers;
    for (int c = 0; c < kConsumerCount; ++c) {
        consumers.emplace_back([&] {
            while (poppedCount.fetch_add(1) < kTotalItems) {
                const int value = queue.Pop();
                std::lock_guard<std::mutex> lock(resultMutex);
                received.insert(value);
            }
        });
    }

    for (auto& t : producers) {
        t.join();
    }
    for (auto& t : consumers) {
        t.join();
    }

    EXPECT_EQ(received.size(), static_cast<std::set<int>::size_type>(kTotalItems));
    for (int i = 0; i < kTotalItems; ++i) {
        EXPECT_TRUE(received.count(i)) << "missing item: " << i;
    }
}
