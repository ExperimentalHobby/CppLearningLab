#include "thread_safe_counter.h"

namespace concurrency {

void ThreadSafeCounter::Increment() {
    std::lock_guard<std::mutex> lock(mutex_);
    ++value_;
}

int64_t ThreadSafeCounter::Value() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return value_;
}

}  // namespace concurrency
