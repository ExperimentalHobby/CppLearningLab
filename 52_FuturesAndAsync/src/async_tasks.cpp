#include "async_tasks.h"

#include <stdexcept>

namespace async_ops {

std::future<int> ComputeSquareAsync(int x) {
    return std::async(std::launch::async, [x] { return x * x; });
}

std::future<std::thread::id> GetExecutionThreadId(std::launch policy) {
    return std::async(policy, [] { return std::this_thread::get_id(); });
}

std::future<int> ThrowingTaskAsync() {
    return std::async(std::launch::async, []() -> int {
        throw std::runtime_error("非同期タスク内で例外が発生しました");
    });
}

std::future<int> RunAddViaPackagedTask(int a, int b) {
    std::packaged_task<int()> task([a, b] { return a + b; });
    std::future<int> future = task.get_future();
    std::thread(std::move(task)).detach();
    return future;
}

}  // namespace async_ops
