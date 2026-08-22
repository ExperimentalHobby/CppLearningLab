// ScopedTransaction: SQLiteのトランザクションをRAIIで管理するガードクラス。
//
// コンストラクタで`BEGIN TRANSACTION`を発行し、Commit()が呼ばれないまま
// スコープを抜ける(正常終了・例外どちらでも)場合はデストラクタが自動的に
// `ROLLBACK`する。04_PointersAndMemoryの`ScopedResource`や
// 07_ExceptionHandlingで扱った「例外発生時も後始末を保証する」RAIIパターンの
// トランザクション版。
#pragma once

#include <stdexcept>
#include <string>

struct sqlite3;

namespace bank {

class TransactionError : public std::runtime_error {
   public:
    explicit TransactionError(const std::string& message) : std::runtime_error(message) {}
};

class ScopedTransaction {
   public:
    explicit ScopedTransaction(sqlite3* db);
    ~ScopedTransaction();

    ScopedTransaction(const ScopedTransaction&) = delete;
    ScopedTransaction& operator=(const ScopedTransaction&) = delete;

    // 明示的にコミットする。呼ばなければデストラクタでロールバックされる。
    void Commit();

   private:
    sqlite3* db_;
    bool committed_ = false;
};

}  // namespace bank
