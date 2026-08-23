// 23. プリペアドステートメントとSQLインジェクション対策
//
// 同じ「ユーザー名でユーザーを検索する」操作を、
//   1. 文字列連結でSQLを組み立てる「脆弱な」実装(FindByUsernameUnsafe)
//   2. プリペアドステートメント+バインドを使う「安全な」実装(FindByUsernameSafe)
// の両方で行い、SQLインジェクション攻撃文字列を与えたときの挙動の違いを
// 実際に確認する。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <vector>

#include "user_repository.h"

namespace {

void PrintUsers(const std::vector<security::User>& users) {
    if (users.empty()) {
        std::cout << "  (該当なし)\n";
        return;
    }
    for (const security::User& user : users) {
        std::cout << "  #" << user.id << " " << user.username << " / password=" << user.password << "\n";
    }
}

void RunCase(const security::UserRepository& repo, const std::string& title, const std::string& input) {
    std::cout << "=== " << title << " (入力: \"" << input << "\") ===\n";

    std::cout << "[脆弱(文字列連結)] SELECT ... WHERE username = '" << input << "'\n";
    try {
        PrintUsers(repo.FindByUsernameUnsafe(input));
    } catch (const security::UserRepositoryError& e) {
        std::cout << "  エラー: " << e.what() << "\n";
    }

    std::cout << "[安全(プリペアドステートメント)] SELECT ... WHERE username = ?\n";
    try {
        PrintUsers(repo.FindByUsernameSafe(input));
    } catch (const security::UserRepositoryError& e) {
        std::cout << "  エラー: " << e.what() << "\n";
    }
    std::cout << "\n";
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    security::UserRepository repo;
    try {
        repo.OpenInMemoryAndSeed();
    } catch (const security::UserRepositoryError& e) {
        std::cerr << "初期化に失敗しました: " << e.what() << "\n";
        return 1;
    }

    std::cout << "23. プリペアドステートメントとSQLインジェクション対策\n\n";

    // 通常の入力: Unsafe/Safeどちらも同じ1件がヒットする。
    RunCase(repo, "通常のユーザー名", "alice");

    // 存在しないユーザー名: どちらも0件。
    RunCase(repo, "存在しないユーザー名", "nobody");

    // 古典的なSQLインジェクション攻撃文字列。
    // 脆弱版は生成されるSQLの意味そのものを書き換えられ、全ユーザー(3件)が
    // 漏洩する(認証チェックのバイパスに相当)。安全版はusernameという文字列に
    // 一致する行を探すだけなので該当なしのまま安全に処理される。
    RunCase(repo, "SQLインジェクション攻撃文字列", "' OR '1'='1");

    std::cout << "結論: 入力値のエスケープではなく、プリペアドステートメント/\n"
                 "パラメータバインディングによってSQLの構造と値を分離することが\n"
                 "SQLインジェクションへの本質的な対策になる。\n";

    return 0;
}
