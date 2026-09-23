// 55. デッドロックの発生と回避
//
// 複数mutexの取得順序に起因するデッドロックと、その回避方法3種類を
// デモで確認する。
#include <windows.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "transfer.h"

namespace {

// workをdetachした別スレッドで実行し、doneフラグがtimeout以内にtrueに
// なればtrueを返す(test/transfer_test.cppのRunWithTimeout()と同じ仕組み)。
bool RunWithTimeout(std::function<void()> work, std::chrono::milliseconds timeout) {
    auto done = std::make_shared<std::atomic<bool>>(false);
    std::thread([work = std::move(work), done] {
        work();
        done->store(true);
    }).detach();

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!done->load()) {
        if (std::chrono::steady_clock::now() > deadline) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return true;
}

void RunOppositeDirectionDemo(const std::string& label,
                               const std::function<void(concurrency::BankAccount&, concurrency::BankAccount&)>&
                                   transfer,
                               std::chrono::milliseconds timeout) {
    auto a = std::make_shared<concurrency::BankAccount>(100);
    auto b = std::make_shared<concurrency::BankAccount>(100);

    const bool completed = RunWithTimeout(
        [a, b, transfer] {
            std::thread t1([&] { transfer(*a, *b); });
            std::thread t2([&] { transfer(*b, *a); });
            t1.join();
            t2.join();
        },
        timeout);

    std::cout << "[" << label << "] "
              << (completed ? "完了しました(デッドロックなし)" : "タイムアウトしました(デッドロック発生)")
              << "\n";
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "=== デモ1: TransferNaive(素朴な実装、デッドロックの再現) ===\n";
    RunOppositeDirectionDemo(
        "TransferNaive",
        [](concurrency::BankAccount& from, concurrency::BankAccount& to) {
            concurrency::TransferNaive(from, to, 10);
        },
        std::chrono::milliseconds(500));

    std::cout << "\n=== デモ2: TransferSafe(std::scoped_lockによる回避) ===\n";
    RunOppositeDirectionDemo(
        "TransferSafe",
        [](concurrency::BankAccount& from, concurrency::BankAccount& to) {
            concurrency::TransferSafe(from, to, 10);
        },
        std::chrono::milliseconds(2000));

    std::cout << "\n=== デモ3: TransferWithTimeout(タイムアウト検知+リトライによる回避) ===\n";
    RunOppositeDirectionDemo(
        "TransferWithTimeout",
        [](concurrency::BankAccount& from, concurrency::BankAccount& to) {
            concurrency::TransferWithTimeout(from, to, 10, std::chrono::milliseconds(50));
        },
        std::chrono::milliseconds(3000));

    return 0;
}
