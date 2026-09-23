#include "lock_free_stack.h"

#include <gtest/gtest.h>

#include <atomic>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

using namespace concurrency;

TEST(LockFreeStackTest, PushThenPopReturnsSameValue) {
    LockFreeStack<int> stack;
    stack.Push(42);

    EXPECT_EQ(stack.Pop(), std::optional<int>(42));
}

TEST(LockFreeStackTest, PopFromEmptyStackReturnsNullopt) {
    LockFreeStack<int> stack;

    EXPECT_EQ(stack.Pop(), std::nullopt);
}

TEST(LockFreeStackTest, MaintainsLifoOrderSingleThreaded) {
    LockFreeStack<int> stack;
    stack.Push(1);
    stack.Push(2);
    stack.Push(3);

    EXPECT_EQ(stack.Pop(), std::optional<int>(3));
    EXPECT_EQ(stack.Pop(), std::optional<int>(2));
    EXPECT_EQ(stack.Pop(), std::optional<int>(1));
    EXPECT_EQ(stack.Pop(), std::nullopt);
}

// 複数スレッドが同時にPush()しても、件数・値が失われないことを確認する。
TEST(LockFreeStackTest, MultipleThreadsPushAllValuesWithoutLoss) {
    constexpr int kThreadCount = 8;
    constexpr int kPushesPerThread = 2000;
    constexpr int kTotal = kThreadCount * kPushesPerThread;

    LockFreeStack<int> stack;
    std::vector<std::thread> threads;
    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([&stack, t, kPushesPerThread] {
            for (int i = 0; i < kPushesPerThread; ++i) {
                stack.Push(t * kPushesPerThread + i);
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }

    std::set<int> popped;
    for (std::optional<int> value; (value = stack.Pop()).has_value();) {
        popped.insert(*value);
    }

    EXPECT_EQ(popped.size(), static_cast<size_t>(kTotal));
    for (int i = 0; i < kTotal; ++i) {
        EXPECT_TRUE(popped.count(i)) << "missing value: " << i;
    }
}

// 2件Pushされたスタックに対し、2スレッドが同時にそれぞれ1回だけPop()を
// 呼んでも、CASが競合した場合に内部でリトライするため、両方とも値を
// 取得できることを確認する(1回でも失敗を諦めてしまう実装だと、
// 競合が起きた試行で片方がnulloptになってしまう)。
TEST(LockFreeStackTest, PopRetriesInternallyWhenCasIsContended) {
    constexpr int kIterations = 300;
    int bothSucceededCount = 0;

    for (int iter = 0; iter < kIterations; ++iter) {
        LockFreeStack<int> stack;
        stack.Push(1);
        stack.Push(2);

        std::atomic<bool> start{false};
        std::optional<int> result1;
        std::optional<int> result2;
        std::thread t1([&] {
            while (!start.load()) {
            }
            result1 = stack.Pop();
        });
        std::thread t2([&] {
            while (!start.load()) {
            }
            result2 = stack.Pop();
        });
        start.store(true);
        t1.join();
        t2.join();

        if (result1.has_value() && result2.has_value()) {
            ++bothSucceededCount;
        }
    }

    EXPECT_EQ(bothSucceededCount, kIterations)
        << "CAS競合時にリトライしていないと、一部の試行で片方がnulloptになる";
}

// 複数producer×複数consumerで、送信した全アイテムが重複・欠落なく届くことを
// 確認する(スレッドセーフ性の核心)。
TEST(LockFreeStackTest, ConcurrentPushAndPopDeliverAllValuesExactlyOnce) {
    constexpr int kProducerCount = 4;
    constexpr int kItemsPerProducer = 2000;
    constexpr int kConsumerCount = 3;
    constexpr int kTotalItems = kProducerCount * kItemsPerProducer;

    LockFreeStack<int> stack;
    std::vector<std::thread> producers;
    for (int p = 0; p < kProducerCount; ++p) {
        producers.emplace_back([&stack, p, kItemsPerProducer] {
            for (int i = 0; i < kItemsPerProducer; ++i) {
                stack.Push(p * kItemsPerProducer + i);
            }
        });
    }

    std::mutex resultMutex;
    std::set<int> received;
    std::atomic<int> reservedCount{0};
    std::vector<std::thread> consumers;
    for (int c = 0; c < kConsumerCount; ++c) {
        consumers.emplace_back([&] {
            while (reservedCount.fetch_add(1) < kTotalItems) {
                std::optional<int> value;
                while (!(value = stack.Pop())) {
                }
                std::lock_guard<std::mutex> lock(resultMutex);
                received.insert(*value);
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
