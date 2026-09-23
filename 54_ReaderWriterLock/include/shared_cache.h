// std::shared_mutexによる読み書きロック付きキーバリューストア。
// 09番のThreadSafeCounter(std::mutex)と異なり、複数の読み取りスレッドを
// 同時に許可しつつ、書き込みスレッドとは排他する。
#pragma once

#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace concurrency {

class SharedCache {
   public:
    void Write(const std::string& key, int value);
    std::optional<int> Read(const std::string& key) const;

    // テスト専用: 共有ロック(読み取り用)を保持した状態でhookを実行する。
    // ロックの粒度(どの区間が「読み取り中」なのか)を外部のテストコードから
    // 観測できるようにするためのフック。
    template <typename F>
    void ReadWithLockHeld(F&& hook) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        hook();
    }

    // テスト専用: 排他ロック(書き込み用)を保持した状態でhookを実行する。
    template <typename F>
    void WriteWithLockHeld(F&& hook) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        hook();
    }

   private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, int> data_;
};

}  // namespace concurrency
