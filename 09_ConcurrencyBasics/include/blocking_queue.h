// std::mutex+std::condition_variableによるスレッドセーフなブロッキングキュー。
// producer(Push)/consumer(Pop)パターンで、キューが空の間Pop()を呼び出した
// スレッドをブロックし、他のスレッドがPush()した時点で起床させる。
//
// クラステンプレートのため、08_TemplatesGenericProgrammingのFixedStack/
// FixedQueueと同様にヘッダーオンリーで実装する(実際に使われた型で
// インスタンス化されて初めてコードが実体化するため)。
#pragma once

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <utility>

namespace concurrency {

template <typename T>
class BlockingQueue {
   public:
    // 値をキューの末尾に追加し、Pop()で待機中のスレッドを1つ起床させる。
    void Push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push_back(std::move(value));
        }
        cv_.notify_one();
    }

    // キューが空の間はPush()されるまでブロックし、先頭の値を取り出して返す。
    T Pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        T value = std::move(queue_.front());
        queue_.pop_front();
        return value;
    }

    // キューが空の間は最大timeoutだけ待ち、それでも値が来なければ
    // std::nulloptを返す。単体テストでPop()相当の待機が実際に発生した
    // ことを検証するために用意している(詳細はPop()のコメントを参照)。
    template <typename Rep, typename Period>
    std::optional<T> TryPopFor(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, timeout, [this] { return !queue_.empty(); })) {
            return std::nullopt;
        }
        T value = std::move(queue_.front());
        queue_.pop_front();
        return value;
    }

    std::size_t Size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

   private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<T> queue_;
};

}  // namespace concurrency
