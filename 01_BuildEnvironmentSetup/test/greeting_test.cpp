// GetGreetingMessage()の単体テスト。
//
// 01番はmain()が1行のHello Worldだが、GoogleTest+CMakeの配線パターンを
// 確立することが目的のため、ロジックをgreeting.h/cppに分離してテストする。
#include <gtest/gtest.h>

#include "greeting.h"

// パス条件: GetGreetingMessage()を呼ぶと、01番の成果物イメージ通り
// "Hello, C++ Learning Lab!" が返ること
TEST(GreetingTest, ReturnsHelloCppLearningLabMessage) {
    EXPECT_EQ(GetGreetingMessage(), "Hello, C++ Learning Lab!");
}
