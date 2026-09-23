#include "transfer.h"

#include <mutex>
#include <thread>

namespace concurrency {

void TransferNaive(BankAccount& from, BankAccount& to, int amount) {
    std::lock_guard<std::timed_mutex> lockFrom(from.GetMutex());
    // 逆方向の同時送金で確実にデッドロックを再現するための仕込み。
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::lock_guard<std::timed_mutex> lockTo(to.GetMutex());
    from.ApplyDeltaUnlocked(-amount);
    to.ApplyDeltaUnlocked(amount);
}

void TransferSafe(BankAccount& from, BankAccount& to, int amount) {
    // std::scoped_lockは複数mutexをデッドロックしないアルゴリズムで
    // 原子的に取得するため、from/toどちらの順で呼ばれても安全。
    std::scoped_lock lock(from.GetMutex(), to.GetMutex());
    from.ApplyDeltaUnlocked(-amount);
    to.ApplyDeltaUnlocked(amount);
}

bool TransferWithTimeout(BankAccount& from, BankAccount& to, int amount,
                          std::chrono::milliseconds timeout) {
    for (;;) {
        std::unique_lock<std::timed_mutex> lockFrom(from.GetMutex(), timeout);
        if (!lockFrom.owns_lock()) {
            return false;
        }
        std::unique_lock<std::timed_mutex> lockTo(to.GetMutex(), std::try_to_lock);
        if (!lockTo.owns_lock()) {
            // 2つ目が取れなかった場合、1つ目も手放してリトライする
            // (デッドロックを検知して諦め、やり直すアプローチ)。
            lockFrom.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        from.ApplyDeltaUnlocked(-amount);
        to.ApplyDeltaUnlocked(amount);
        return true;
    }
}

}  // namespace concurrency
