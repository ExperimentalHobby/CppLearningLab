#include "scoped_transaction.h"

#include <gtest/gtest.h>
#include <sqlite3.h>

#include <stdexcept>

using bank::ScopedTransaction;

namespace {

// テストごとに独立したインメモリDB+1行だけのテーブルを用意するヘルパー。
class ScopedTransactionTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ASSERT_EQ(sqlite3_open(":memory:", &db_), SQLITE_OK);
        ASSERT_EQ(sqlite3_exec(db_, "CREATE TABLE t (value INTEGER);", nullptr, nullptr, nullptr),
                   SQLITE_OK);
    }

    void TearDown() override { sqlite3_close(db_); }

    int CountRows() const {
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM t;", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        const int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }

    sqlite3* db_ = nullptr;
};

}  // namespace

// Commit()を呼べば、スコープを抜けた後も変更が残る。
TEST_F(ScopedTransactionTest, CommitPersistsChanges) {
    {
        ScopedTransaction transaction(db_);
        sqlite3_exec(db_, "INSERT INTO t (value) VALUES (1);", nullptr, nullptr, nullptr);
        transaction.Commit();
    }

    EXPECT_EQ(CountRows(), 1);
}

// Commit()を呼ばずにスコープを抜けると、デストラクタが自動的にROLLBACKし
// 変更が残らない(RAIIによる後始末保証)。
TEST_F(ScopedTransactionTest, DestructorRollsBackWithoutCommit) {
    {
        ScopedTransaction transaction(db_);
        sqlite3_exec(db_, "INSERT INTO t (value) VALUES (1);", nullptr, nullptr, nullptr);
        // Commit()を呼ばずにスコープを抜ける。
    }

    EXPECT_EQ(CountRows(), 0);
}

// 例外がスコープを通り抜けて巻き戻る場合も、デストラクタは必ず呼ばれるため
// ROLLBACKされる。
TEST_F(ScopedTransactionTest, RollsBackWhenExceptionUnwindsScope) {
    try {
        ScopedTransaction transaction(db_);
        sqlite3_exec(db_, "INSERT INTO t (value) VALUES (1);", nullptr, nullptr, nullptr);
        throw std::runtime_error("boom");
    } catch (const std::runtime_error&) {
        // 例外はここで飲み込む。ScopedTransactionのデストラクタは既に呼ばれているはず。
    }

    EXPECT_EQ(CountRows(), 0);
}
