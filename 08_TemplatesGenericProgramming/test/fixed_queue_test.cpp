#include "fixed_queue.h"

#include <gtest/gtest.h>

TEST(FixedQueueTest, StartsEmpty) {
    FixedQueue<int, 4> queue;

    EXPECT_TRUE(queue.Empty());
    EXPECT_FALSE(queue.Full());
    EXPECT_EQ(queue.Size(), 0u);
}

// FIFO(先入れ先出し)であることを確認する。
TEST(FixedQueueTest, EnqueueAndDequeueFollowFifoOrder) {
    FixedQueue<int, 4> queue;
    queue.Enqueue(1);
    queue.Enqueue(2);
    queue.Enqueue(3);

    EXPECT_EQ(queue.Front(), 1);
    EXPECT_EQ(queue.Dequeue(), 1);
    EXPECT_EQ(queue.Dequeue(), 2);
    EXPECT_EQ(queue.Dequeue(), 3);
    EXPECT_TRUE(queue.Empty());
}

// リングバッファのhead/tailが容量を超えて折り返しても正しく動作することを確認する
// (Dequeueで空きを作ってからEnqueueし、内部インデックスを一周させる)。
TEST(FixedQueueTest, WrapsAroundRingBufferCorrectly) {
    FixedQueue<int, 3> queue;
    queue.Enqueue(1);
    queue.Enqueue(2);
    queue.Dequeue();       // head_が1つ進む
    queue.Enqueue(3);
    queue.Enqueue(4);      // tail_が容量を超えて折り返す

    EXPECT_EQ(queue.Dequeue(), 2);
    EXPECT_EQ(queue.Dequeue(), 3);
    EXPECT_EQ(queue.Dequeue(), 4);
}

TEST(FixedQueueTest, ThrowsWhenEnqueueingBeyondCapacity) {
    FixedQueue<int, 1> queue;
    queue.Enqueue(1);

    EXPECT_THROW(queue.Enqueue(2), std::out_of_range);
}

TEST(FixedQueueTest, ThrowsWhenDequeueingEmptyQueue) {
    FixedQueue<int, 4> queue;

    EXPECT_THROW(queue.Dequeue(), std::out_of_range);
    EXPECT_THROW(queue.Front(), std::out_of_range);
}
