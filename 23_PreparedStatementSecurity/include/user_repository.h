// UserRepository: SQLインジェクションの危険性とプリペアドステートメントによる
// 対策を並べて実演するための最小限のユーザー検索リポジトリ。
//
// FindByUsernameUnsafe()は意図的に文字列連結でSQLを組み立てる「脆弱な」実装、
// FindByUsernameSafe()はプレースホルダ+バインドを使う「安全な」実装。
// 本番コードでは決してUnsafe版のような実装をしないこと。
#pragma once

#include <stdexcept>
#include <string>
#include <vector>

struct sqlite3;

namespace security {

class UserRepositoryError : public std::runtime_error {
   public:
    explicit UserRepositoryError(const std::string& message) : std::runtime_error(message) {}
};

struct User {
    long long id = 0;
    std::string username;
    std::string password;
};

class UserRepository {
   public:
    UserRepository() = default;
    ~UserRepository();

    UserRepository(const UserRepository&) = delete;
    UserRepository& operator=(const UserRepository&) = delete;

    // インメモリSQLite DBを開き、テーブル作成+サンプルユーザーのシードを行う。
    void OpenInMemoryAndSeed();

    // 【脆弱】文字列連結でSQLを組み立てて実行する。usernameに
    // `' OR '1'='1`のような文字列を与えるとSQL構造そのものが書き換わり、
    // 意図しない全件取得などが起こりうる。
    std::vector<User> FindByUsernameUnsafe(const std::string& username) const;

    // 【安全】`?`プレースホルダ+`sqlite3_bind_text`でパラメータをバインドする。
    // usernameの中身がどんな文字列であってもSQLの構造は変化しない。
    std::vector<User> FindByUsernameSafe(const std::string& username) const;

   private:
    sqlite3* db_ = nullptr;

    [[noreturn]] void ThrowLastError(const std::string& context) const;
};

}  // namespace security
