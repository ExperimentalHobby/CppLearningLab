#include "atomic_max.h"

namespace concurrency {

void UpdateMaxAtomic(std::atomic<int>& target, int candidate, const std::function<void()>& afterLoadHook) {
    int current = target.load(std::memory_order_relaxed);
    if (afterLoadHook) {
        afterLoadHook();
    }
    // compare_exchange_weak(current, candidate)は「targetが今もcurrentの
    // ままなら原子的にcandidateへ書き換え、trueを返す」「他スレッドの
    // 割り込みでtargetが既に変わっていればfalseを返し、currentを最新値
    // に更新する(参照渡し)」という動作をする。falseが返った場合は
    // 最新のcurrentで改めてcandidate > currentを判定し直すため、
    // 「読んだ時より大きい値に他スレッドが更新済みなら上書きしない」が
    // 保証され、load()とstore()を分割した素朴な実装のようなTOCTOUが
    // 起きない。
    while (candidate > current) {
        if (target.compare_exchange_weak(current, candidate, std::memory_order_relaxed)) {
            return;
        }
    }
}

}  // namespace concurrency
