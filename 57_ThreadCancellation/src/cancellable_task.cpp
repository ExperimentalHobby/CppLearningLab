#include "cancellable_task.h"

namespace concurrency {

CancellableTask::CancellableTask(std::function<bool()> workUnit, std::chrono::milliseconds pollInterval)
    : workUnit_(std::move(workUnit)), pollInterval_(pollInterval) {}

CancellableTask::~CancellableTask() {
    RequestCancel();
    Join();
}

void CancellableTask::Start() {
    thread_ = std::thread([this] { Run(); });
}

void CancellableTask::RequestCancel() {
    cancelRequested_.store(true);
    // 待機中のワーカースレッドを即座に起床させ、pollIntervalの満了を
    // 待たずにキャンセル要求へ応答できるようにする。
    cv_.notify_all();
}

bool CancellableTask::WaitForCompletion(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return completionCv_.wait_for(lock, timeout, [this] { return finished_.load(); });
}

void CancellableTask::Join() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

void CancellableTask::Run() {
    for (;;) {
        if (cancelRequested_.load()) {
            wasCancelled_.store(true);
            break;
        }
        const bool shouldContinue = workUnit_();
        if (!shouldContinue) {
            break;
        }
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, pollInterval_, [this] { return cancelRequested_.load(); });
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_.store(true);
    }
    completionCv_.notify_all();
}

}  // namespace concurrency
