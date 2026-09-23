#include "atomic_counter.h"

namespace concurrency {

void AtomicCounter::Increment() {
    // fetch_add()は「読み取り→加算→書き戻し」を1つの原子操作として行う。
    // カウンタの加算順序自体には意味が無く、他の変数の読み書きとの
    // 前後関係を保証する必要も無いため、memory_order_relaxedで十分。
    value_.fetch_add(1, std::memory_order_relaxed);
}

int64_t AtomicCounter::Value() const {
    return value_.load(std::memory_order_relaxed);
}

}  // namespace concurrency
