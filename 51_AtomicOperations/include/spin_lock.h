// std::atomic_flagによる自作ロック(スピンロック)。std::mutexと違い
// OSのブロッキング待機ではなく、フラグが取れるまでループで待ち続ける
// (busy-wait)。ロック区間が極めて短い場合はスレッド切り替えのコストを
// 避けられる一方、区間が長いとCPUを無駄に消費し続ける欠点がある。
//
// memory_orderの明示がこのクラスの学習ポイント: lock()の
// test_and_set(memory_order_acquire)は「これより後のこのスレッドの
// 読み書きが、lock()より前に先読みされない」ことを保証し、unlock()の
// clear(memory_order_release)は「これより前のこのスレッドの読み書きが、
// 次にlock()できた別スレッドから必ず見える」ことを保証する。この
// acquire/releaseの対応関係により、非atomicな共有変数をロック区間内で
// 読み書きしても、他スレッドから見て一貫した値が見える。
#pragma once

#include <atomic>

namespace concurrency {

class SpinLock {
   public:
    void lock();
    void unlock();

   private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

}  // namespace concurrency
