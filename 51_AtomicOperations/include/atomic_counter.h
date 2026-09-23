// std::atomic<int64_t>のfetch_add()によるロックフリーカウンタ。
// 09番のThreadSafeCounter(std::mutexで保護)と同じ「複数スレッドから
// 同時にIncrement()しても更新が欠落しない」性質を、ロックを使わずに
// 実現する比較対象。カウンタの加算順序自体には意味が無いため、
// memory_order_relaxedで十分(他の変数の読み書きとの前後関係を
// 保証する必要が無い)。
#pragma once

#include <atomic>
#include <cstdint>

namespace concurrency {

class AtomicCounter {
   public:
    void Increment();
    int64_t Value() const;

   private:
    std::atomic<int64_t> value_{0};
};

}  // namespace concurrency
