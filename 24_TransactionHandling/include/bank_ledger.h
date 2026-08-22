// BankLedger: 「口座Aから口座Bへ送金する」処理を題材に、トランザクションによる
// データ整合性の担保を実演するための最小限の口座台帳。
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

struct sqlite3;

namespace bank {

class BankLedgerError : public std::runtime_error {
   public:
    explicit BankLedgerError(const std::string& message) : std::runtime_error(message) {}
};

// 残高不足など、業務的な理由で送金できない場合に投げる。
class InsufficientFundsError : public BankLedgerError {
   public:
    explicit InsufficientFundsError(const std::string& message) : BankLedgerError(message) {}
};

class BankLedger {
   public:
    BankLedger() = default;
    ~BankLedger();

    BankLedger(const BankLedger&) = delete;
    BankLedger& operator=(const BankLedger&) = delete;

    void OpenInMemoryAndSeed();

    // 口座の残高を取得する。
    int64_t GetBalance(int64_t accountId) const;

    // fromIdからtoIdへamountを送金する。
    //
    // 出金→入金の2ステップを1つのトランザクションにまとめる。出金時に残高が
    // 不足していればInsufficientFundsErrorを投げてトランザクション全体を
    // ロールバックする(呼び出し側で追加の後始末は不要)ため、失敗しても
    // 両口座の残高は送金前の状態のまま保たれる。
    void Transfer(int64_t fromId, int64_t toId, int64_t amount);

   private:
    sqlite3* db_ = nullptr;

    void Withdraw(int64_t accountId, int64_t amount);
    void Deposit(int64_t accountId, int64_t amount);
};

}  // namespace bank
