// 54. 読み書きロック(std::shared_mutex)
//
// std::shared_mutexによる読み書きロックを、2つのデモで確認する。
#include <windows.h>

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "shared_cache.h"

namespace {

// デモ1: 複数のreaderが同時にロックへ入れることを確認する。
void RunConcurrentReadersDemo() {
    constexpr int kReaderCount = 4;
    concurrency::SharedCache cache;
    cache.Write("key", 100);

    std::mutex coutMutex;
    std::vector<std::thread> readers;
    for (int i = 0; i < kReaderCount; ++i) {
        readers.emplace_back([&cache, &coutMutex, i] {
            cache.ReadWithLockHeld([&] {
                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << "[reader" << i << "] 読み取り開始\n";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << "[reader" << i << "] 読み取り終了\n";
                }
            });
        });
    }
    for (auto& t : readers) {
        t.join();
    }
    std::cout << "[ConcurrentReaders] " << kReaderCount << "人のreaderがほぼ同時に実行されました"
                 "(開始ログが全員分先に出るはず)\n";
}

// デモ2: writer実行中はreaderも待たされることを確認する。
void RunWriterExclusionDemo() {
    concurrency::SharedCache cache;
    std::mutex coutMutex;

    std::thread writer([&cache, &coutMutex] {
        cache.WriteWithLockHeld([&] {
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "[writer] 書き込み開始(200ms)\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "[writer] 書き込み終了\n";
            }
        });
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::thread reader([&cache, &coutMutex] {
        cache.ReadWithLockHeld([&] {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "[reader] writer終了後に読み取りを開始できました\n";
        });
    });

    writer.join();
    reader.join();
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "=== デモ1: 複数readerの同時アクセス ===\n";
    RunConcurrentReadersDemo();

    std::cout << "\n=== デモ2: writer実行中のreader排他 ===\n";
    RunWriterExclusionDemo();

    return 0;
}
