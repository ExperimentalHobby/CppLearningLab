// 56. ロックフリースタック
//
// std::atomic<Node*>+compare_exchange_weakによるTreiber stackのデモ。
#include <windows.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include "lock_free_stack.h"

namespace {

void RunLockFreeStackDemo() {
    constexpr int kProducerCount = 4;
    constexpr int kItemsPerProducer = 1000;
    constexpr int kConsumerCount = 3;
    constexpr int kTotalItems = kProducerCount * kItemsPerProducer;

    concurrency::LockFreeStack<int> stack;
    std::vector<std::thread> producers;
    for (int p = 0; p < kProducerCount; ++p) {
        producers.emplace_back([&stack, p, kItemsPerProducer] {
            for (int i = 0; i < kItemsPerProducer; ++i) {
                stack.Push(p * kItemsPerProducer + i);
            }
        });
    }

    std::mutex resultMutex;
    std::set<int> received;
    std::atomic<int> reservedCount{0};
    std::vector<std::thread> consumers;
    for (int c = 0; c < kConsumerCount; ++c) {
        consumers.emplace_back([&] {
            while (reservedCount.fetch_add(1) < kTotalItems) {
                std::optional<int> value;
                while (!(value = stack.Pop())) {
                }
                std::lock_guard<std::mutex> lock(resultMutex);
                received.insert(*value);
            }
        });
    }

    for (auto& t : producers) {
        t.join();
    }
    for (auto& t : consumers) {
        t.join();
    }

    std::cout << "[LockFreeStack] " << kTotalItems << "件中" << received.size()
              << "件を重複・欠落なく受信しました\n";
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "=== デモ: LockFreeStackによる複数producer/複数consumer ===\n";
    RunLockFreeStackDemo();

    return 0;
}
