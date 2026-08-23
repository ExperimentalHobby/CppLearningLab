#include "user_repository.h"

#include <sqlite3.h>

namespace security {

namespace {

// sqlite3_execのコールバックで受け取った1行をUserに変換してvectorへ積む。
int CollectUserCallback(void* context, int columnCount, char** columnValues, char** /*columnNames*/) {
    auto* users = static_cast<std::vector<User>*>(context);
    User user;
    if (columnCount > 0 && columnValues[0] != nullptr) {
        user.id = std::stoll(columnValues[0]);
    }
    if (columnCount > 1 && columnValues[1] != nullptr) {
        user.username = columnValues[1];
    }
    if (columnCount > 2 && columnValues[2] != nullptr) {
        user.password = columnValues[2];
    }
    users->push_back(std::move(user));
    return 0;
}

}  // namespace

UserRepository::~UserRepository() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
    }
}

void UserRepository::ThrowLastError(const std::string& context) const {
    throw UserRepositoryError(context + ": " + (db_ != nullptr ? sqlite3_errmsg(db_) : "unknown"));
}

void UserRepository::OpenInMemoryAndSeed() {
    // ":memory:"でファイルを作らずインメモリDBを使う(このデモは永続化不要のため)。
    if (sqlite3_open(":memory:", &db_) != SQLITE_OK) {
        throw UserRepositoryError("インメモリDBを開けませんでした");
    }

    char* errMsg = nullptr;
    constexpr const char* kSetup =
        "CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT NOT NULL, password TEXT NOT NULL);"
        "INSERT INTO users (id, username, password) VALUES"
        " (1, 'alice', 'alice-pass'),"
        " (2, 'bob', 'bob-pass'),"
        " (3, 'admin', 'super-secret');";
    if (sqlite3_exec(db_, kSetup, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        const std::string message = std::string("初期データの投入に失敗しました: ") + (errMsg != nullptr ? errMsg : "");
        sqlite3_free(errMsg);
        throw UserRepositoryError(message);
    }
}

std::vector<User> UserRepository::FindByUsernameUnsafe(const std::string& username) const {
    // 【脆弱】ユーザー入力をそのままSQL文字列に埋め込んでいる。
    // usernameに"' OR '1'='1"を渡すと、実際に実行されるSQLは
    //   SELECT id, username, password FROM users WHERE username = '' OR '1'='1'
    // となり、WHERE句が常に真になって全行が返ってしまう。
    const std::string sql =
        "SELECT id, username, password FROM users WHERE username = '" + username + "';";

    std::vector<User> users;
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), CollectUserCallback, &users, &errMsg) != SQLITE_OK) {
        const std::string message = std::string("検索に失敗しました: ") + (errMsg != nullptr ? errMsg : "");
        sqlite3_free(errMsg);
        throw UserRepositoryError(message);
    }
    return users;
}

std::vector<User> UserRepository::FindByUsernameSafe(const std::string& username) const {
    // 【安全】`?`はSQLの構文要素ではなく単なる「値の差し込み口」として扱われる。
    // usernameにどんな文字列(引用符やSQLキーワードを含む文字列)を渡しても、
    // SQLの構造(WHERE句の意味)自体は変化しない。
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, "SELECT id, username, password FROM users WHERE username = ?;", -1, &stmt,
                            nullptr) != SQLITE_OK) {
        ThrowLastError("プリペアドステートメントの作成に失敗しました");
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    std::vector<User> users;
    int rc = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        User user;
        user.id = sqlite3_column_int64(stmt, 0);
        const unsigned char* nameText = sqlite3_column_text(stmt, 1);
        user.username = nameText != nullptr ? reinterpret_cast<const char*>(nameText) : "";
        const unsigned char* passwordText = sqlite3_column_text(stmt, 2);
        user.password = passwordText != nullptr ? reinterpret_cast<const char*>(passwordText) : "";
        users.push_back(std::move(user));
    }
    const bool failed = rc != SQLITE_DONE;
    sqlite3_finalize(stmt);
    if (failed) {
        ThrowLastError("検索に失敗しました");
    }
    return users;
}

}  // namespace security
