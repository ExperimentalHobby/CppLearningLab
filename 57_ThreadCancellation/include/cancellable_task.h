// C++20のstd::jthread/std::stop_tokenを使わず、std::atomic<bool>の
// キャンセルフラグ+std::condition_variableで実現する協調的キャンセル
// (cooperative cancellation)。強制終了ではなく、ワーカースレッド自身が
// 定期的にキャンセル要求を確認して自発的に終了する。
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace concurrency {

class CancellableTask {
   public:
    // workUnitがfalseを返すか、キャンセルが要求されるまで、workUnitを
    // pollInterval間隔で繰り返し呼び出す。
    CancellableTask(std::function<bool()> workUnit, std::chrono::milliseconds pollInterval);
    ~CancellableTask();

    CancellableTask(const CancellableTask&) = delete;
    CancellableTask& operator=(const CancellableTask&) = delete;

    void Start();

    // キャンセルを要求する。待機中のワーカースレッドはpollIntervalの満了を
    // 待たずに即座に起床し、キャンセル要求を確認して終了する。
    void RequestCancel();

    // タスクの終了(正常終了/キャンセルによる終了のどちらか)をtimeout以内に
    // 待つ。終了していればtrue、まだ実行中ならfalseを返す。
    bool WaitForCompletion(std::chrono::milliseconds timeout);

    void Join();
    bool WasCancelled() const { return wasCancelled_.load(); }

   private:
    void Run();

    std::function<bool()> workUnit_;
    std::chrono::milliseconds pollInterval_;
    std::atomic<bool> cancelRequested_{false};
    std::atomic<bool> wasCancelled_{false};
    std::atomic<bool> finished_{false};
    std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable completionCv_;
    std::thread thread_;
};

}  // namespace concurrency
