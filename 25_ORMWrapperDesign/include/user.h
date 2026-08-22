// User エンティティと、Repository<User, UserMapper>が要求するインターフェースを
// 実装するUserMapper。
#pragma once

#include <sqlite3.h>

#include <string>

namespace orm {

struct User {
    long long id = 0;
    std::string name;
    std::string email;
};

struct UserMapper {
    static std::string TableName() { return "users"; }

    static std::string CreateTableSql() {
        return "CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "name TEXT NOT NULL,"
               "email TEXT NOT NULL"
               ");";
    }

    static std::string SelectColumnsSql() { return "name, email"; }
    static int ColumnCount() { return 2; }

    static void BindInsertParams(sqlite3_stmt* stmt, const User& user) {
        sqlite3_bind_text(stmt, 1, user.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, user.email.c_str(), -1, SQLITE_TRANSIENT);
    }

    static void BindUpdateParams(sqlite3_stmt* stmt, const User& user) {
        sqlite3_bind_text(stmt, 1, user.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, user.email.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, user.id);
    }

    static User FromStatement(sqlite3_stmt* stmt) {
        User user;
        user.id = sqlite3_column_int64(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        user.name = name != nullptr ? reinterpret_cast<const char*>(name) : "";
        const unsigned char* email = sqlite3_column_text(stmt, 2);
        user.email = email != nullptr ? reinterpret_cast<const char*>(email) : "";
        return user;
    }

    static long long GetId(const User& user) { return user.id; }
    static void SetId(User& user, long long id) { user.id = id; }
};

}  // namespace orm
