// デッドロック実演用の単純な口座クラス。std::timed_mutexを使うのは、
// transfer.hのTransferWithTimeout()がtry_lock_for()によるタイムアウト付き
// ロックを必要とするため(通常のstd::mutexにはtry_lock_forが無い)。
// std::timed_mutexは通常のlock()/unlock()もサポートするため、
// TransferNaive()/TransferSafe()もこのクラスをそのまま使える。
#pragma once

#include <mutex>

namespace concurrency {

class BankAccount {
   public:
    explicit BankAccount(int initialBalance) : balance_(initialBalance) {}

    int Balance() const {
        std::lock_guard<std::timed_mutex> lock(mutex_);
        return balance_;
    }

    std::timed_mutex& GetMutex() { return mutex_; }

    // 呼び出し元が既にGetMutex()をロック済みであることを前提とする、
    // ロックなしでの残高変更。transfer.h側の各Transfer関数から使う。
    void ApplyDeltaUnlocked(int delta) { balance_ += delta; }

   private:
    mutable std::timed_mutex mutex_;
    int balance_;
};

}  // namespace concurrency
