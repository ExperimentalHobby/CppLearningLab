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
// 値を返すことを確認する。別スレッド側で意図的に少し待ってからPush()する
// ことで、「先にPop()が呼ばれて待機状態に入っている」状況を作る。
TEST(BlockingQueueTest, PopBlocksUntilItemIsPushed) {
    BlockingQueue<int> queue;

    std::thread producer([&queue] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.Push(7);
    });

    EXPECT_EQ(queue.Pop(), 7);
    producer.join();
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

    EXPECT_EQ(received.size(), static_cast<size_t>(kTotalItems));
    for (int i = 0; i < kTotalItems; ++i) {
        EXPECT_TRUE(received.count(i)) << "missing item: " << i;
    }
}
