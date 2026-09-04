// 24. トランザクション制御
//
// 「口座Aから口座Bへ送金する」処理を題材に、commit/rollbackによる
// データ整合性の担保を実演する。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>

#include "bank_ledger.h"

namespace {

constexpr int64_t kAlice = 1;
constexpr int64_t kBob = 2;

void PrintBalances(const bank::BankLedger& ledger, const char* label) {
    std::cout << label << ": Alice=" << ledger.GetBalance(kAlice) << " Bob=" << ledger.GetBalance(kBob)
              << "\n";
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    bank::BankLedger ledger;
    try {
        ledger.OpenInMemoryAndSeed();
    } catch (const bank::BankLedgerError& e) {
        std::cerr << "初期化に失敗しました: " << e.what() << "\n";
        return 1;
    }

    std::cout << "24. トランザクション制御\n\n";
    PrintBalances(ledger, "初期状態");

    std::cout << "\n--- ケース1: 正常な送金(Alice -> Bob, 300) ---\n";
    try {
        ledger.Transfer(kAlice, kBob, 300);
        std::cout << "送金に成功しました(コミット済み)。\n";
    } catch (const bank::BankLedgerError& e) {
        std::cout << "送金に失敗しました: " << e.what() << "\n";
    }
    PrintBalances(ledger, "送金後");

    std::cout << "\n--- ケース2: 残高不足で失敗する送金(Bob -> Alice, 10000) ---\n";
    try {
        ledger.Transfer(kBob, kAlice, 10000);
        std::cout << "送金に成功しました(コミット済み)。\n";
    } catch (const bank::BankLedgerError& e) {
        std::cout << "送金に失敗しました(トランザクションは自動ロールバック済み): " << e.what() << "\n";
    }
    PrintBalances(ledger, "失敗後");

    std::cout << "\n--- ケース3: 出金は成功するが入金先が存在せず失敗する送金(Alice -> id=999, 100) ---\n";
    try {
        ledger.Transfer(kAlice, 999, 100);
        std::cout << "送金に成功しました(コミット済み)。\n";
    } catch (const bank::BankLedgerError& e) {
        std::cout << "送金に失敗しました(トランザクションは自動ロールバック済み): " << e.what() << "\n";
    }
    PrintBalances(ledger, "失敗後");

    std::cout << "\nケース2・3のいずれも、Transfer内で例外が発生した時点でScopedTransactionの\n"
                 "デストラクタが自動的にROLLBACKする。ケース3ではWithdraw(出金)自体は\n"
                 "一度DB上で実行済みだったにもかかわらず、コミット前にロールバックされる\n"
                 "ため、Aliceの残高はケース1直後から変化していない。「複数の操作を\n"
                 "ひとまとまりとして扱う」トランザクションの効果が確認できる。\n";

    return 0;
}
