// Repository<T, Mapper>: テーブルとC++オブジェクトを対応付ける簡易ORM風の
// 汎用リポジトリ。
//
// 08_TemplatesGenericProgrammingで扱ったテンプレート技法の応用として、
// エンティティ固有の知識(テーブル名・列名・SQL・バインド方法)を全て
// 「Mapperポリシークラス」側に閉じ込め、Repository本体はSQLを一切知らない
// 汎用的なfind/save/removeだけを提供する設計にしている。
//
// Mapper<T>に要求するインターフェース(コンセプトの明文化。C++17なので
// concepts言語機能は使わず、コンパイル時ダックタイピングで表現する):
//   static std::string TableName();
//   static std::string CreateTableSql();
//   静的メンバ関数として以下を持つこと:
//     static std::string SelectColumnsSql();                       // "name, email" 等(idを除く)
//     static int ColumnCount();                                    // 上記の列数(INSERT用プレースホルダ数)
//     static void BindInsertParams(sqlite3_stmt*, const T&);       // idを除く列をバインド
//     static void BindUpdateParams(sqlite3_stmt*, const T&);       // idを除く列+最後にidをバインド
//     static T FromStatement(sqlite3_stmt*);                       // SELECT結果の1行からTを構築
//     static long long GetId(const T&);
//     static void SetId(T&, long long);
#pragma once

#include <sqlite3.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace orm {

class RepositoryError : public std::runtime_error {
   public:
    explicit RepositoryError(const std::string& message) : std::runtime_error(message) {}
};

namespace detail {

// RAIIでsqlite3_stmtを後始末する。repository.h内でのみ使う実装詳細。
class Statement {
   public:
    Statement(sqlite3* db, const std::string& sql) {
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_, nullptr) != SQLITE_OK) {
            throw RepositoryError("プリペアドステートメントの作成に失敗しました: " +
                                   std::string(sqlite3_errmsg(db)));
        }
    }
    ~Statement() { sqlite3_finalize(stmt_); }

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    sqlite3_stmt* get() const { return stmt_; }

   private:
    sqlite3_stmt* stmt_ = nullptr;
};

}  // namespace detail

template <typename T, typename Mapper>
class Repository {
   public:
    explicit Repository(sqlite3* db) : db_(db) {}

    // テーブルが無ければ作成する。
    void EnsureSchema() {
        char* errMsg = nullptr;
        const std::string sql = Mapper::CreateTableSql();
        if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            const std::string message =
                "テーブル作成に失敗しました(" + Mapper::TableName() + "): " + (errMsg != nullptr ? errMsg : "");
            sqlite3_free(errMsg);
            throw RepositoryError(message);
        }
    }

    std::vector<T> FindAll() const {
        const std::string sql =
            "SELECT id, " + Mapper::SelectColumnsSql() + " FROM " + Mapper::TableName() + " ORDER BY id ASC;";
        detail::Statement stmt(db_, sql);
        std::vector<T> items;
        int rc = 0;
        while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
            items.push_back(Mapper::FromStatement(stmt.get()));
        }
        if (rc != SQLITE_DONE) {
            throw RepositoryError("一覧取得に失敗しました(" + Mapper::TableName() + "): " +
                                   sqlite3_errmsg(db_));
        }
        return items;
    }

    bool FindById(long long id, T& out) const {
        const std::string sql =
            "SELECT id, " + Mapper::SelectColumnsSql() + " FROM " + Mapper::TableName() + " WHERE id = ?;";
        detail::Statement stmt(db_, sql);
        sqlite3_bind_int64(stmt.get(), 1, id);
        const int rc = sqlite3_step(stmt.get());
        if (rc == SQLITE_ROW) {
            out = Mapper::FromStatement(stmt.get());
            return true;
        }
        if (rc != SQLITE_DONE) {
            throw RepositoryError("検索に失敗しました(" + Mapper::TableName() + "): " + sqlite3_errmsg(db_));
        }
        return false;
    }

    // Mapper::GetId(entity)が0ならINSERT(採番されたidをentityに書き戻す)、
    // それ以外はUPDATEする。いずれの場合もentityのidを返す。
    long long Save(T& entity) const {
        if (Mapper::GetId(entity) == 0) {
            const std::string sql = "INSERT INTO " + Mapper::TableName() + " (" + Mapper::SelectColumnsSql() +
                                     ") VALUES (" + Placeholders(Mapper::ColumnCount()) + ");";
            detail::Statement stmt(db_, sql);
            Mapper::BindInsertParams(stmt.get(), entity);
            if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
                throw RepositoryError("追加に失敗しました(" + Mapper::TableName() + "): " +
                                       sqlite3_errmsg(db_));
            }
            const long long newId = sqlite3_last_insert_rowid(db_);
            Mapper::SetId(entity, newId);
            return newId;
        }

        const std::string sql =
            "UPDATE " + Mapper::TableName() + " SET " + AssignmentsSql() + " WHERE id = ?;";
        detail::Statement stmt(db_, sql);
        Mapper::BindUpdateParams(stmt.get(), entity);
        if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
            throw RepositoryError("更新に失敗しました(" + Mapper::TableName() + "): " + sqlite3_errmsg(db_));
        }
        return Mapper::GetId(entity);
    }

    bool Remove(long long id) const {
        const std::string sql = "DELETE FROM " + Mapper::TableName() + " WHERE id = ?;";
        detail::Statement stmt(db_, sql);
        sqlite3_bind_int64(stmt.get(), 1, id);
        if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
            throw RepositoryError("削除に失敗しました(" + Mapper::TableName() + "): " + sqlite3_errmsg(db_));
        }
        return sqlite3_changes(db_) > 0;
    }

   private:
    sqlite3* db_;

    // "?, ?, ?" のようなプレースホルダ列を組み立てる。
    static std::string Placeholders(int count) {
        std::string result;
        for (int i = 0; i < count; ++i) {
            if (i > 0) {
                result += ", ";
            }
            result += "?";
        }
        return result;
    }

    // SelectColumnsSql()の"name, email"から"name = ?, email = ?"を組み立てる。
    static std::string AssignmentsSql() {
        const std::string columns = Mapper::SelectColumnsSql();
        std::string result;
        std::string current;
        auto flush = [&]() {
            if (!current.empty()) {
                if (!result.empty()) {
                    result += ", ";
                }
                result += current + " = ?";
                current.clear();
            }
        };
        for (const char c : columns) {
            if (c == ',') {
                flush();
            } else if (c != ' ') {
                current += c;
            }
        }
        flush();
        return result;
    }
};

}  // namespace orm
