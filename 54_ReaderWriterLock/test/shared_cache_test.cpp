#include "shared_cache.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

using namespace concurrency;

TEST(SharedCacheTest, WriteThenReadReturnsValue) {
    SharedCache cache;
    cache.Write("a", 42);

    EXPECT_EQ(cache.Read("a"), std::optional<int>(42));
}

TEST(SharedCacheTest, ReadMissingKeyReturnsNullopt) {
    SharedCache cache;

    EXPECT_EQ(cache.Read("missing"), std::nullopt);
}

// 複数のreaderが同時にロック内へ入れることを、「今何人が同時に中にいるか」の
// 最大値で確認する。std::shared_lockであれば全員が同時に入れるはずなので、
// 最大同時人数はreader数と一致する。std::unique_lock(排他ロック)だと
// readerが直列化され、最大同時人数は常に1になる。
TEST(SharedCacheTest, MultipleReadersRunConcurrently) {
    constexpr int kReaderCount = 8;
    SharedCache cache;
    std::atomic<int> currentInside{0};
    std::atomic<int> maxObserved{0};

    std::vector<std::thread> readers;
    for (int i = 0; i < kReaderCount; ++i) {
        readers.emplace_back([&cache, &currentInside, &maxObserved, kReaderCount] {
            cache.ReadWithLockHeld([&] {
                const int nowInside = currentInside.fetch_add(1) + 1;
                int prevMax = maxObserved.load();
                while (nowInside > prevMax && !maxObserved.compare_exchange_weak(prevMax, nowInside)) {
                }

                const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
                while (currentInside.load() < kReaderCount) {
                    if (std::chrono::steady_clock::now() > deadline) {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                currentInside.fetch_sub(1);
            });
        });
    }
    for (auto& t : readers) {
        t.join();
    }

    EXPECT_EQ(maxObserved.load(), kReaderCount);
}

// writerが排他ロックを保持している間は、readerがそのタイミングでロックに
// 入れない(=書き込み中であることを示すフラグがtrueの状態を観測しない)
// ことを確認する。
TEST(SharedCacheTest, WriterExcludesReadersWhileWriting) {
    constexpr int kReaderCount = 8;
    SharedCache cache;
    std::atomic<bool> writing{false};
    std::atomic<bool> writerEntered{false};
    std::atomic<int> readersObservedWriting{0};

    std::thread writer([&cache, &writing, &writerEntered] {
        cache.WriteWithLockHeld([&] {
            writing.store(true);
            writerEntered.store(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            writing.store(false);
        });
    });

    while (!writerEntered.load()) {
    }

    std::vector<std::thread> readers;
    for (int i = 0; i < kReaderCount; ++i) {
        readers.emplace_back([&cache, &writing, &readersObservedWriting] {
            cache.ReadWithLockHeld([&] {
                if (writing.load()) {
                    readersObservedWriting.fetch_add(1);
                }
            });
        });
    }
    for (auto& t : readers) {
        t.join();
    }
    writer.join();

    EXPECT_EQ(readersObservedWriting.load(), 0);
}
