#include "async_tasks.h"

#include <gtest/gtest.h>

#include <future>
#include <stdexcept>
#include <thread>

using namespace async_ops;

TEST(AsyncTasksTest, ComputeSquareAsyncReturnsCorrectValue) {
    EXPECT_EQ(ComputeSquareAsync(5).get(), 25);
    EXPECT_EQ(ComputeSquareAsync(-3).get(), 9);
    EXPECT_EQ(ComputeSquareAsync(0).get(), 0);
}

// std::launch::deferredは呼び出し元がget()を呼んだ時点で、呼び出し元自身の
// スレッド上で遅延実行される。
TEST(AsyncTasksTest, DeferredPolicyRunsOnCallingThreadWhenGetIsCalled) {
    std::future<std::thread::id> future = GetExecutionThreadId(std::launch::deferred);
    const std::thread::id callerId = std::this_thread::get_id();

    EXPECT_EQ(future.get(), callerId);
}

// std::launch::asyncは呼び出し時点で即座に別スレッド上で実行される。
TEST(AsyncTasksTest, AsyncPolicyRunsOnDifferentThread) {
    std::future<std::thread::id> future = GetExecutionThreadId(std::launch::async);
    const std::thread::id callerId = std::this_thread::get_id();

    EXPECT_NE(future.get(), callerId);
}

// std::asyncで実行したタスクが投げた例外は、future::get()を呼んだ側の
// スレッドで再送出される。
TEST(AsyncTasksTest, ThrowingTaskPropagatesExceptionThroughFutureGet) {
    std::future<int> future = ThrowingTaskAsync();

    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST(AsyncTasksTest, PackagedTaskReturnsCorrectSum) {
    EXPECT_EQ(RunAddViaPackagedTask(3, 4).get(), 7);
    EXPECT_EQ(RunAddViaPackagedTask(-1, 1).get(), 0);
}
