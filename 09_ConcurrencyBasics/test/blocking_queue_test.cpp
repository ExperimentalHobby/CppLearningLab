#include "blocking_queue.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <set>
#include <thread>
#include <vector>

using namespace concurrency;

TEST(BlockingQueueTest, PushThenPopReturnsSameValue) {
    BlockingQueue<int> queue;

    queue.Push(42);

    EXPECT_EQ(queue.Pop(), 42);
}

// Size()は08番のFixedQueueTest.StartsEmptyと同様、空・Push後・Pop後の
// 状態を単体で検証する(Push/Popの戻り値だけを見るテストでは、Size()の
// 実装やロックが壊れていても検出できないため)。
TEST(BlockingQueueTest, SizeReflectsEmptyPushAndPopStates) {
    BlockingQueue<int> queue;

    EXPECT_EQ(queue.Size(), 0u);

    queue.Push(1);
    queue.Push(2);
    EXPECT_EQ(queue.Size(), 2u);

    queue.Pop();
    EXPECT_EQ(queue.Size(), 1u);

    queue.Pop();
    EXPECT_EQ(queue.Size(), 0u);
}

// Pop()はキューが空の間ブロックすることを確認したい。しかし「別スレッドで
// Pop()を呼び、一定時間待っても完了していなければブロックしているとみなす」
// という判定方法は、判定側スレッドがconsumerスレッドの実行タイミングに
// 依存してしまう。consumerスレッドが「Pop()を呼び出した」ことを示す
// フラグを立てた直後にOSにスケジューリングを奪われ、その間にメイン
// スレッド側の判定が済んでしまった場合、非ブロッキングな実装であっても
// 判定をすり抜けてしまう可能性がある(PR #97のCopilotレビュー指摘)。
//
// このレースを構造的に無くすため、待機自体を判定を行うのと同じ
// スレッドの中で行うTryPopFor()(内部的にはPop()と同じcv_.wait系の
// 待機ロジックを使う)を用い、ウォールクロック時間の実測で
// 「指定時間分、実際に待たされたこと」を検証する。他スレッドの
// スケジューリングに依存しないため、判定は決定的になる。
TEST(BlockingQueueTest, TryPopForActuallyWaitsWhenQueueIsEmpty) {
    BlockingQueue<int> queue;
    constexpr auto kTimeout = std::chrono::milliseconds(100);

    const auto start = std::chrono::steady_clock::now();
    const std::optional<int> result = queue.TryPopFor(kTimeout);
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result.has_value());
    // タイマーの粒度による多少の前倒しは許容しつつ、"即座に返っていない"
    // ことを確認する(即座に返る実装ならelapsedはkTimeoutよりずっと短くなる)。
    EXPECT_GE(elapsed, kTimeout - std::chrono::milliseconds(20))
        << "TryPopFor()が指定時間待たずに即座に返ってしまった(ブロックしていない)";
}

// TryPopFor()はタイムアウト前にPush()されれば、待機を打ち切って正しい
// 値を返すことを確認する。
TEST(BlockingQueueTest, TryPopForReturnsValueWhenPushedWithinTimeout) {
    BlockingQueue<int> queue;

    std::thread producer([&queue] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        queue.Push(99);
    });

    const std::optional<int> result = queue.TryPopFor(std::chrono::milliseconds(500));
    producer.join();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 99);
}

// Pop()自体も、別スレッドから遅れてPush()された値を正しく受け取れる
// ことを確認する(ブロック性自体は上記TryPopForのテストで検証済み。
// Pop()とTryPopFor()は同じ待機条件を共有しているため、Pop()についても
// 同様にブロックすることの傍証になる)。
TEST(BlockingQueueTest, PopReturnsValuePushedFromAnotherThread) {
    BlockingQueue<int> queue;

    std::thread producer([&queue] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
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

    EXPECT_EQ(received.size(), static_cast<std::set<int>::size_type>(kTotalItems));
    for (int i = 0; i < kTotalItems; ++i) {
        EXPECT_TRUE(received.count(i)) << "missing item: " << i;
    }
}
