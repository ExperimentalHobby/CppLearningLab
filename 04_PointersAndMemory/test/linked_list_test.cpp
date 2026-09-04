#include "linked_list.h"

#include <gtest/gtest.h>

TEST(RawIntListTest, StartsEmpty) {
    RawIntList list;
    EXPECT_EQ(list.Size(), 0u);
    EXPECT_EQ(list.ToString(), "[]");
}

// PushFrontは先頭に追加するため、後から追加した値ほど先頭に来る(逆順)。
TEST(RawIntListTest, PushFrontAddsToFrontInReverseOrder) {
    RawIntList list;
    list.PushFront(1);
    list.PushFront(2);
    list.PushFront(3);

    EXPECT_EQ(list.Size(), 3u);
    EXPECT_EQ(list.ToString(), "[3, 2, 1]");
}

TEST(SmartIntListTest, StartsEmpty) {
    SmartIntList list;
    EXPECT_EQ(list.Size(), 0u);
    EXPECT_EQ(list.ToString(), "[]");
}

TEST(SmartIntListTest, PushFrontAddsToFrontInReverseOrder) {
    SmartIntList list;
    list.PushFront(1);
    list.PushFront(2);
    list.PushFront(3);

    EXPECT_EQ(list.Size(), 3u);
    EXPECT_EQ(list.ToString(), "[3, 2, 1]");
}
