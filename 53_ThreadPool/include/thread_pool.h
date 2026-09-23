// 固定数のワーカースレッドとタスクキューによるスレッドプール。
//
// 09番のBlockingQueue<T>は「stopを通知して待機を打ち切る」機能を持たない
// ため、終了処理(残タスクの完了待ち)が必要なこのクラスでは専用の
// タスクキュー(mutex+condition_variable)を内部に持つ。
// 52番のstd::packaged_task+std::futureのパターンを応用し、Enqueue()した
// タスクの戻り値を呼び出し元がfutureで受け取れるようにする。
#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace async_ops {

class ThreadPool {
   public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 以降のEnqueue()を拒否し、キューに残っているタスクの完了を待って
    // からワーカースレッドをjoinする。デストラクタからも呼ばれるため、
    // 明示的に呼ばなくても安全に破棄できる。
    void Shutdown();

    template <typename F, typename... Args>
    auto Enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<ReturnType> future = task->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_) {
                throw std::runtime_error("ThreadPoolは終了処理済みのためタスクを投入できません");
            }
            tasks_.emplace_back([task] { (*task)(); });
        }
        cv_.notify_one();
        return future;
    }

   private:
    void WorkerLoop();

    std::vector<std::thread> workers_;
    std::deque<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;
};

}  // namespace async_ops
