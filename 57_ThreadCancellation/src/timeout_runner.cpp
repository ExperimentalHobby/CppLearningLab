#include "timeout_runner.h"

namespace concurrency {

bool RunWithTimeout(CancellableTask& task, std::chrono::milliseconds timeout) {
    task.Start();
    if (task.WaitForCompletion(timeout)) {
        task.Join();
        return true;
    }
    task.RequestCancel();
    task.Join();
    return false;
}

}  // namespace concurrency
