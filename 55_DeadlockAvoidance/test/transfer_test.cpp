#include "transfer.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

using namespace concurrency;

namespace {

// workをdetachした別スレッドで実行し、doneフラグがtimeout以内にtrueに
// なればtrueを返す。detach()しているため、workが万一デッドロックしても
// (そのスレッド自体は残り続けるが)テストプロセスが停止することはなく、
// timeout経過後にfalseを返して制御を戻せる。
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

// 2口座A/Bに対し、A→BとB→Aの逆方向送金を同時に走らせるヘルパー。
void RunOppositeDirectionTransfers(BankAccount& a, BankAccount& b,
                                    const std::function<void(BankAccount&, BankAccount&)>& transfer) {
    std::thread t1([&] { transfer(a, b); });
    std::thread t2([&] { transfer(b, a); });
    t1.join();
    t2.join();
}

}  // namespace

TEST(TransferTest, TransferNaiveAppliesCorrectBalancesSingleThreaded) {
    BankAccount a(100);
    BankAccount b(50);

    TransferNaive(a, b, 30);

    EXPECT_EQ(a.Balance(), 70);
    EXPECT_EQ(b.Balance(), 80);
}

TEST(TransferTest, TransferSafeAppliesCorrectBalancesSingleThreaded) {
    BankAccount a(100);
    BankAccount b(50);

    TransferSafe(a, b, 30);

    EXPECT_EQ(a.Balance(), 70);
    EXPECT_EQ(b.Balance(), 80);
}

TEST(TransferTest, TransferWithTimeoutAppliesCorrectBalancesSingleThreaded) {
    BankAccount a(100);
    BankAccount b(50);

    EXPECT_TRUE(TransferWithTimeout(a, b, 30, std::chrono::milliseconds(100)));
    EXPECT_EQ(a.Balance(), 70);
    EXPECT_EQ(b.Balance(), 80);
}

// TransferNaive()は、逆方向の同時送金でデッドロックすることを実際に確認する
// (デッドロックの再現)。
TEST(TransferTest, TransferNaiveDeadlocksUnderOppositeDirectionContention) {
    auto a = std::make_shared<BankAccount>(100);
    auto b = std::make_shared<BankAccount>(100);

    const bool completed = RunWithTimeout(
        [a, b] {
            RunOppositeDirectionTransfers(*a, *b, [](BankAccount& from, BankAccount& to) {
                TransferNaive(from, to, 10);
            });
        },
        std::chrono::milliseconds(500));

    EXPECT_FALSE(completed) << "TransferNaive()がデッドロックせず完了してしまった";
}

// std::scoped_lockによるTransferSafe()は、同じ状況でもデッドロックしない
// ことを確認する。
TEST(TransferTest, TransferSafeDoesNotDeadlockUnderOppositeDirectionContention) {
    auto a = std::make_shared<BankAccount>(100);
    auto b = std::make_shared<BankAccount>(100);

    const bool completed = RunWithTimeout(
        [a, b] {
            RunOppositeDirectionTransfers(*a, *b, [](BankAccount& from, BankAccount& to) {
                TransferSafe(from, to, 10);
            });
        },
        std::chrono::milliseconds(2000));

    EXPECT_TRUE(completed) << "TransferSafe()がデッドロックしてタイムアウトした";
}

// タイムアウト付きロック+リトライのTransferWithTimeout()も、同じ状況で
// デッドロックしないことを確認する。
TEST(TransferTest, TransferWithTimeoutDoesNotDeadlockUnderOppositeDirectionContention) {
    auto a = std::make_shared<BankAccount>(100);
    auto b = std::make_shared<BankAccount>(100);

    const bool completed = RunWithTimeout(
        [a, b] {
            RunOppositeDirectionTransfers(*a, *b, [](BankAccount& from, BankAccount& to) {
                TransferWithTimeout(from, to, 10, std::chrono::milliseconds(50));
            });
        },
        std::chrono::milliseconds(3000));

    EXPECT_TRUE(completed) << "TransferWithTimeout()がデッドロックしてタイムアウトした";
}

// 多数スレッドがTransferSafe()で送金し合っても、2口座の残高合計は
// 最初の合計から変わらないことを確認する(スレッドセーフ性の核心)。
TEST(TransferTest, TransferSafePreservesTotalBalanceUnderConcurrentTransfers) {
    constexpr int kThreadCount = 8;
    constexpr int kTransfersPerThread = 500;
    BankAccount a(10000);
    BankAccount b(10000);
    const int expectedTotal = a.Balance() + b.Balance();

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&a, &b, i, kTransfersPerThread] {
            for (int j = 0; j < kTransfersPerThread; ++j) {
                if ((i + j) % 2 == 0) {
                    TransferSafe(a, b, 1);
                } else {
                    TransferSafe(b, a, 1);
                }
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(a.Balance() + b.Balance(), expectedTotal);
}
