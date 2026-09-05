#include "bank_ledger.h"

#include <gtest/gtest.h>

using bank::BankLedger;
using bank::InsufficientFundsError;

// BankLedgerはコピー・ムーブのいずれも不可(sqlite3*を所有するため)なので、
// ヘルパー関数で生成して返すのではなく、各テストで直接スタック上に構築する。

TEST(BankLedgerTest, SeedsInitialBalances) {
    BankLedger ledger;
    ledger.OpenInMemoryAndSeed();

    EXPECT_EQ(ledger.GetBalance(1), 1000);
    EXPECT_EQ(ledger.GetBalance(2), 500);
}

TEST(BankLedgerTest, TransferMovesAmountBetweenAccounts) {
    BankLedger ledger;
    ledger.OpenInMemoryAndSeed();

    ledger.Transfer(1, 2, 300);

    EXPECT_EQ(ledger.GetBalance(1), 700);
    EXPECT_EQ(ledger.GetBalance(2), 800);
}

// 残高不足時は例外を投げ、出金(Withdraw)・入金(Deposit)のどちらも
// 反映されない(トランザクション全体がロールバックされる)ことを確認する。
TEST(BankLedgerTest, TransferRollsBackOnInsufficientFunds) {
    BankLedger ledger;
    ledger.OpenInMemoryAndSeed();

    EXPECT_THROW(ledger.Transfer(1, 2, 2000), InsufficientFundsError);

    EXPECT_EQ(ledger.GetBalance(1), 1000);
    EXPECT_EQ(ledger.GetBalance(2), 500);
}

TEST(BankLedgerTest, GetBalanceThrowsForUnknownAccount) {
    BankLedger ledger;
    ledger.OpenInMemoryAndSeed();

    EXPECT_THROW(ledger.GetBalance(999), bank::BankLedgerError);
}
