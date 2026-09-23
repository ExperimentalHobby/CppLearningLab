// std::mutexで保護されたカウンタ。複数スレッドから同時にIncrement()しても
// 更新の欠落(lost update)が起きないことを保証する。
#pragma once

#include <cstdint>
#include <mutex>

namespace concurrency {

class ThreadSafeCounter {
   public:
    void Increment();
    int64_t Value() const;

   private:
    mutable std::mutex mutex_;
    int64_t value_ = 0;
};

}  // namespace concurrency
