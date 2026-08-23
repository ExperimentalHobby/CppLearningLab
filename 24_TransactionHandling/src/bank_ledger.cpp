#include "bank_ledger.h"

#include <sqlite3.h>

#include "scoped_transaction.h"

namespace bank {

namespace {

class Statement {
   public:
    Statement(sqlite3* db, const char* sql) {
        if (sqlite3_prepare_v2(db, sql, -1, &stmt_, nullptr) != SQLITE_OK) {
            throw BankLedgerError(std::string("プリペアドステートメントの作成に失敗しました: ") +
                                   sqlite3_errmsg(db));
        }
    }
    ~Statement() { sqlite3_finalize(stmt_); }

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    sqlite3_stmt* get() const { return stmt_; }

   private:
    sqlite3_stmt* stmt_ = nullptr;
};

}  // namespace

BankLedger::~BankLedger() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
    }
}

void BankLedger::OpenInMemoryAndSeed() {
    if (sqlite3_open(":memory:", &db_) != SQLITE_OK) {
        throw BankLedgerError("インメモリDBを開けませんでした");
    }
    char* errMsg = nullptr;
    constexpr const char* kSetup =
        "CREATE TABLE accounts (id INTEGER PRIMARY KEY, name TEXT NOT NULL, balance INTEGER NOT NULL);"
        "INSERT INTO accounts (id, name, balance) VALUES (1, 'Alice', 1000), (2, 'Bob', 500);";
    if (sqlite3_exec(db_, kSetup, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        const std::string message = std::string("初期データの投入に失敗しました: ") + (errMsg != nullptr ? errMsg : "");
        sqlite3_free(errMsg);
        throw BankLedgerError(message);
    }
}

int64_t BankLedger::GetBalance(int64_t accountId) const {
    Statement stmt(db_, "SELECT balance FROM accounts WHERE id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, accountId);
    if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
        throw BankLedgerError("口座が見つかりません: id=" + std::to_string(accountId));
    }
    return sqlite3_column_int64(stmt.get(), 0);
}

void BankLedger::Withdraw(int64_t accountId, int64_t amount) {
    const int64_t balance = GetBalance(accountId);
    if (balance < amount) {
        throw InsufficientFundsError("残高不足です: id=" + std::to_string(accountId) +
                                      " balance=" + std::to_string(balance) +
                                      " amount=" + std::to_string(amount));
    }
    Statement stmt(db_, "UPDATE accounts SET balance = balance - ? WHERE id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, amount);
    sqlite3_bind_int64(stmt.get(), 2, accountId);
    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw BankLedgerError(std::string("出金に失敗しました: ") + sqlite3_errmsg(db_));
    }
}

void BankLedger::Deposit(int64_t accountId, int64_t amount) {
    Statement stmt(db_, "UPDATE accounts SET balance = balance + ? WHERE id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, amount);
    sqlite3_bind_int64(stmt.get(), 2, accountId);
    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw BankLedgerError(std::string("入金に失敗しました: ") + sqlite3_errmsg(db_));
    }
    // 存在しない口座IDを指定した場合、UPDATE自体は0行更新のままSQLITE_DONEで
    // 成功してしまう(SQLの仕様上エラーにはならない)。ここで明示的に検知しないと
    // 「Withdrawだけ成功してDepositは何も起きない」まま処理が正常終了したように
    // 見えてしまうため、影響行数を確認して業務的なエラーとして扱う。
    if (sqlite3_changes(db_) == 0) {
        throw BankLedgerError("入金先の口座が見つかりません: id=" + std::to_string(accountId));
    }
}

void BankLedger::Transfer(int64_t fromId, int64_t toId, int64_t amount) {
    // ScopedTransactionはこの関数のどのパスで抜けても(正常return、例外)
    // 後始末が保証される。Commit()を呼ばずに関数を抜ければ自動的にROLLBACK
    // されるため、Withdraw()が例外を投げた場合にDeposit()を実行してしまう
    // (整合性が崩れる)心配がない。
    ScopedTransaction transaction(db_);
    Withdraw(fromId, amount);
    Deposit(toId, amount);
    transaction.Commit();
}

}  // namespace bank
