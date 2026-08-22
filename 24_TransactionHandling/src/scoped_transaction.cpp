#include "scoped_transaction.h"

#include <sqlite3.h>

namespace bank {

namespace {

void Exec(sqlite3* db, const char* sql, const char* context) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        const std::string message = std::string(context) + ": " + (errMsg != nullptr ? errMsg : "");
        sqlite3_free(errMsg);
        throw TransactionError(message);
    }
}

}  // namespace

ScopedTransaction::ScopedTransaction(sqlite3* db) : db_(db) {
    Exec(db_, "BEGIN TRANSACTION;", "トランザクション開始に失敗しました");
}

ScopedTransaction::~ScopedTransaction() {
    if (!committed_) {
        // ここに来るのは、Commit()が呼ばれる前に例外でスコープを抜けた場合、
        // または呼び出し側が意図的にCommit()を呼ばなかった場合。
        // デストラクタから例外は投げられないため、ROLLBACK自体の失敗は
        // 握りつぶす(致命的だがログ出力すらできない状況は稀という判断)。
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
}

void ScopedTransaction::Commit() {
    Exec(db_, "COMMIT;", "コミットに失敗しました");
    committed_ = true;
}

}  // namespace bank
