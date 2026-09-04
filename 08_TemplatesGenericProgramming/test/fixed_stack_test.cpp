#include "fixed_stack.h"

#include <gtest/gtest.h>

TEST(FixedStackTest, StartsEmpty) {
    FixedStack<int, 4> stack;

    EXPECT_TRUE(stack.Empty());
    EXPECT_FALSE(stack.Full());
    EXPECT_EQ(stack.Size(), 0u);
}

// LIFO(後入れ先出し)であることを確認する。
TEST(FixedStackTest, PushAndPopFollowLifoOrder) {
    FixedStack<int, 4> stack;
    stack.Push(1);
    stack.Push(2);
    stack.Push(3);

    EXPECT_EQ(stack.Top(), 3);
    EXPECT_EQ(stack.Pop(), 3);
    EXPECT_EQ(stack.Pop(), 2);
    EXPECT_EQ(stack.Pop(), 1);
    EXPECT_TRUE(stack.Empty());
}

TEST(FixedStackTest, BecomesFullAtCapacity) {
    FixedStack<int, 2> stack;
    stack.Push(1);
    stack.Push(2);

    EXPECT_TRUE(stack.Full());
    EXPECT_EQ(stack.Size(), 2u);
}

TEST(FixedStackTest, ThrowsWhenPushingBeyondCapacity) {
    FixedStack<int, 1> stack;
    stack.Push(1);

    EXPECT_THROW(stack.Push(2), std::out_of_range);
}

TEST(FixedStackTest, ThrowsWhenPoppingEmptyStack) {
    FixedStack<int, 4> stack;

    EXPECT_THROW(stack.Pop(), std::out_of_range);
    EXPECT_THROW(stack.Top(), std::out_of_range);
}
