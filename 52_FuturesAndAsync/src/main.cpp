// 52. std::future/std::asyncによる非同期処理
//
// std::async/std::future/std::packaged_taskによる非同期処理を、4つの
// デモで確認する。
#include <windows.h>

#include <iostream>
#include <sstream>

#include "async_tasks.h"

namespace {

// デモ1: std::async(launch::async)で計算した結果をfuture::get()で受け取る。
void RunComputeSquareDemo() {
    std::future<int> future = async_ops::ComputeSquareAsync(7);
    std::cout << "[ComputeSquareAsync] 7の平方=" << future.get() << "\n";
}

// デモ2: launch::deferredとlaunch::asyncで、タスクが実行されるスレッドが
// 異なることを確認する。
void RunLaunchPolicyDemo() {
    const std::thread::id mainThreadId = std::this_thread::get_id();

    std::future<std::thread::id> deferredFuture =
        async_ops::GetExecutionThreadId(std::launch::deferred);
    std::ostringstream deferredIdStream;
    deferredIdStream << deferredFuture.get();
    std::ostringstream mainIdStream;
    mainIdStream << mainThreadId;
    std::cout << "[deferred] メインスレッド=" << mainIdStream.str()
              << " タスク実行スレッド=" << deferredIdStream.str()
              << (deferredIdStream.str() == mainIdStream.str() ? " (一致。呼び出し元で遅延実行された)"
                                                                : " (不一致)")
              << "\n";

    std::future<std::thread::id> asyncFuture = async_ops::GetExecutionThreadId(std::launch::async);
    std::ostringstream asyncIdStream;
    asyncIdStream << asyncFuture.get();
    std::cout << "[async] メインスレッド=" << mainIdStream.str()
              << " タスク実行スレッド=" << asyncIdStream.str()
              << (asyncIdStream.str() != mainIdStream.str() ? " (不一致。別スレッドで実行された)"
                                                             : " (一致)")
              << "\n";
}

// デモ3: std::asyncで実行したタスクが投げた例外が、future::get()側で
// 再送出されることを確認する。
void RunThrowingTaskDemo() {
    std::future<int> future = async_ops::ThrowingTaskAsync();
    try {
        future.get();
        std::cout << "[ThrowingTaskAsync] 例外が送出されませんでした(想定外)\n";
    } catch (const std::exception& e) {
        std::cout << "[ThrowingTaskAsync] future.get()で例外を捕捉: " << e.what() << "\n";
    }
}

// デモ4: std::packaged_taskで「実行するタスク」と「結果の受け取り」を
// 分離する。
void RunPackagedTaskDemo() {
    std::future<int> future = async_ops::RunAddViaPackagedTask(10, 32);
    std::cout << "[RunAddViaPackagedTask] 10+32=" << future.get() << "\n";
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "=== デモ1: std::asyncによる非同期計算 ===\n";
    RunComputeSquareDemo();

    std::cout << "\n=== デモ2: launch::deferred/asyncの違い ===\n";
    RunLaunchPolicyDemo();

    std::cout << "\n=== デモ3: future::get()での例外の再送出 ===\n";
    RunThrowingTaskDemo();

    std::cout << "\n=== デモ4: std::packaged_taskによるタスクと結果の分離 ===\n";
    RunPackagedTaskDemo();

    return 0;
}
