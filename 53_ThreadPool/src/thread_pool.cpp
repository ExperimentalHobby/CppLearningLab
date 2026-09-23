#include "thread_pool.h"

namespace async_ops {

ThreadPool::ThreadPool(size_t numThreads) {
    if (numThreads == 0) {
        // ワーカーが1つも起動されないと、Enqueue()したタスクのfutureが
        // 永久に完了しなくなる(WorkerLoop()を動かす者がいないため)ので、
        // 生成前に拒否する。
        throw std::invalid_argument("ThreadPoolはnumThreads>=1で構築する必要があります");
    }
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this] { WorkerLoop(); });
    }
}

ThreadPool::~ThreadPool() {
    Shutdown();
}

void ThreadPool::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stop_) {
            return;
        }
        stop_ = true;
    }
    cv_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::WorkerLoop() {
    for (;;) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            // stopが立っていても、キューに残っているタスクがあれば処理を
            // 続ける(終了処理での残タスク完了待ちを実現する)。
            if (stop_ && tasks_.empty()) {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop_front();
        }
        task();
    }
}

}  // namespace async_ops
