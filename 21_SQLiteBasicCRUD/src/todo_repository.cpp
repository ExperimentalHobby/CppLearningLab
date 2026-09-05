#include "todo_repository.h"

#include <sqlite3.h>

#include <utility>

namespace todo {

namespace {

// RAIIでsqlite3_stmtを後始末するための小さなラッパー。
class Statement {
   public:
    Statement(sqlite3* db, const char* sql) {
        if (sqlite3_prepare_v2(db, sql, -1, &stmt_, nullptr) != SQLITE_OK) {
            throw TodoRepositoryError(std::string("プリペアドステートメントの作成に失敗しました: ") +
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

TodoRepository::~TodoRepository() { Close(); }

TodoRepository::TodoRepository(TodoRepository&& other) noexcept : db_(other.db_) {
    other.db_ = nullptr;
}

TodoRepository& TodoRepository::operator=(TodoRepository&& other) noexcept {
    if (this != &other) {
        Close();
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

void TodoRepository::ThrowLastError(const std::string& context) const {
    throw TodoRepositoryError(context + ": " + (db_ != nullptr ? sqlite3_errmsg(db_) : "unknown"));
}

void TodoRepository::Open(const std::string& dbPath) {
    Close();
    if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
        const std::string message =
            std::string("DBを開けませんでした: ") + (db_ != nullptr ? sqlite3_errmsg(db_) : "unknown");
        Close();
        throw TodoRepositoryError(message);
    }
    EnsureSchema();
}

void TodoRepository::Close() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void TodoRepository::EnsureSchema() {
    constexpr const char* kCreateTable =
        "CREATE TABLE IF NOT EXISTS todos ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  title TEXT NOT NULL,"
        "  done INTEGER NOT NULL DEFAULT 0"
        ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, kCreateTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        const std::string message = std::string("テーブル作成に失敗しました: ") + (errMsg != nullptr ? errMsg : "");
        sqlite3_free(errMsg);
        throw TodoRepositoryError(message);
    }
}

long long TodoRepository::Add(const std::string& title) {
    Statement stmt(db_, "INSERT INTO todos (title, done) VALUES (?, 0);");
    sqlite3_bind_text(stmt.get(), 1, title.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        ThrowLastError("TODOの追加に失敗しました");
    }
    return sqlite3_last_insert_rowid(db_);
}

std::vector<TodoItem> TodoRepository::List() const {
    Statement stmt(db_, "SELECT id, title, done FROM todos ORDER BY id ASC;");
    std::vector<TodoItem> items;
    int rc = 0;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        TodoItem item;
        item.id = sqlite3_column_int64(stmt.get(), 0);
        const unsigned char* text = sqlite3_column_text(stmt.get(), 1);
        item.title = text != nullptr ? reinterpret_cast<const char*>(text) : "";
        item.done = sqlite3_column_int(stmt.get(), 2) != 0;
        items.push_back(std::move(item));
    }
    if (rc != SQLITE_DONE) {
        ThrowLastError("TODO一覧の取得に失敗しました");
    }
    return items;
}

bool TodoRepository::FindById(long long id, TodoItem& out) const {
    Statement stmt(db_, "SELECT id, title, done FROM todos WHERE id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, id);
    const int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_ROW) {
        out.id = sqlite3_column_int64(stmt.get(), 0);
        const unsigned char* text = sqlite3_column_text(stmt.get(), 1);
        out.title = text != nullptr ? reinterpret_cast<const char*>(text) : "";
        out.done = sqlite3_column_int(stmt.get(), 2) != 0;
        return true;
    }
    if (rc != SQLITE_DONE) {
        ThrowLastError("TODOの検索に失敗しました");
    }
    return false;
}

bool TodoRepository::Update(long long id, const std::string& title, bool done) {
    Statement stmt(db_, "UPDATE todos SET title = ?, done = ? WHERE id = ?;");
    sqlite3_bind_text(stmt.get(), 1, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt.get(), 2, done ? 1 : 0);
    sqlite3_bind_int64(stmt.get(), 3, id);
    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        ThrowLastError("TODOの更新に失敗しました");
    }
    // UPDATE自体はWHERE条件に一致する行が無くてもSQLITE_DONEを返すため、
    // 実際に更新された行数(sqlite3_changes)を見て存在有無を呼び出し元に伝える。
    return sqlite3_changes(db_) > 0;
}

bool TodoRepository::SetDone(long long id, bool done) {
    Statement stmt(db_, "UPDATE todos SET done = ? WHERE id = ?;");
    sqlite3_bind_int(stmt.get(), 1, done ? 1 : 0);
    sqlite3_bind_int64(stmt.get(), 2, id);
    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        ThrowLastError("完了状態の更新に失敗しました");
    }
    return sqlite3_changes(db_) > 0;
}

bool TodoRepository::Remove(long long id) {
    Statement stmt(db_, "DELETE FROM todos WHERE id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, id);
    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        ThrowLastError("TODOの削除に失敗しました");
    }
    return sqlite3_changes(db_) > 0;
}

}  // namespace todo
