#include "spin_lock.h"

namespace concurrency {

void SpinLock::lock() {
    // test_and_set()は「フラグを立てて、立てる前の値を返す」を原子的に
    // 行う。既に他スレッドが立てていれば(true)ループして待ち続け
    // (busy-wait)、自分が立てられた(false→true)ら取得成功。
    // memory_order_acquireにより、これ以降のこのスレッドの読み書きが
    // lock()より前に先読みされないことを保証する。
    while (flag_.test_and_set(std::memory_order_acquire)) {
    }
}

void SpinLock::unlock() {
    // memory_order_releaseにより、これより前のこのスレッドの読み書きが、
    // 次にlock()できた別スレッドのacquireから必ず見えることを保証する。
    flag_.clear(std::memory_order_release);
}

}  // namespace concurrency
