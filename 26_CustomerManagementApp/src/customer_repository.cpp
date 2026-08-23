#include "customer_repository.h"

#include <sqlite3.h>

namespace customer {

namespace {

class Statement {
   public:
    Statement(sqlite3* db, const char* sql) {
        if (sqlite3_prepare_v2(db, sql, -1, &stmt_, nullptr) != SQLITE_OK) {
            throw CustomerRepositoryError(std::string("プリペアドステートメントの作成に失敗しました: ") +
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

Customer RowToCustomer(sqlite3_stmt* stmt) {
    Customer c;
    c.id = sqlite3_column_int64(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    c.name = name != nullptr ? reinterpret_cast<const char*>(name) : "";
    const unsigned char* phone = sqlite3_column_text(stmt, 2);
    c.phone = phone != nullptr ? reinterpret_cast<const char*>(phone) : "";
    const unsigned char* email = sqlite3_column_text(stmt, 3);
    c.email = email != nullptr ? reinterpret_cast<const char*>(email) : "";
    return c;
}

}  // namespace

CustomerRepository::~CustomerRepository() { Close(); }

void CustomerRepository::Open(const std::string& dbPath) {
    Close();
    if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
        const std::string message =
            std::string("DBを開けませんでした: ") + (db_ != nullptr ? sqlite3_errmsg(db_) : "unknown");
        Close();
        throw CustomerRepositoryError(message);
    }
    EnsureSchema();
}

void CustomerRepository::Close() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void CustomerRepository::EnsureSchema() {
    constexpr const char* kCreateTable =
        "CREATE TABLE IF NOT EXISTS customers ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL,"
        "  phone TEXT NOT NULL DEFAULT '',"
        "  email TEXT NOT NULL DEFAULT ''"
        ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, kCreateTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        const std::string message = std::string("テーブル作成に失敗しました: ") + (errMsg != nullptr ? errMsg : "");
        sqlite3_free(errMsg);
        throw CustomerRepositoryError(message);
    }
}

long long CustomerRepository::Add(const Customer& customer) {
    Statement stmt(db_, "INSERT INTO customers (name, phone, email) VALUES (?, ?, ?);");
    sqlite3_bind_text(stmt.get(), 1, customer.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 2, customer.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 3, customer.email.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw CustomerRepositoryError(std::string("追加に失敗しました: ") + sqlite3_errmsg(db_));
    }
    return sqlite3_last_insert_rowid(db_);
}

std::vector<Customer> CustomerRepository::RunQuery(const std::string& sql,
                                                    const std::string& likePattern) const {
    Statement stmt(db_, sql.c_str());
    if (!likePattern.empty()) {
        sqlite3_bind_text(stmt.get(), 1, likePattern.c_str(), -1, SQLITE_TRANSIENT);
    }
    std::vector<Customer> customers;
    int rc = 0;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        customers.push_back(RowToCustomer(stmt.get()));
    }
    if (rc != SQLITE_DONE) {
        throw CustomerRepositoryError(std::string("検索に失敗しました: ") + sqlite3_errmsg(db_));
    }
    return customers;
}

std::vector<Customer> CustomerRepository::List() const {
    return RunQuery("SELECT id, name, phone, email FROM customers ORDER BY id ASC;", "");
}

std::vector<Customer> CustomerRepository::Search(const std::string& nameQuery) const {
    if (nameQuery.empty()) {
        return List();
    }
    // LIKEの'%'/'_'をエスケープしないと、検索文字列自体にこれらが含まれる場合に
    // 意図しないワイルドカードとして解釈されてしまうため、ESCAPE句で'\'を
    // エスケープ文字に指定している。パラメータ自体は`?`バインドで渡すため
    // SQLインジェクションの心配はない(23_PreparedStatementSecurity参照)。
    std::string escaped;
    for (const char c : nameQuery) {
        if (c == '%' || c == '_' || c == '\\') {
            escaped += '\\';
        }
        escaped += c;
    }
    const std::string likePattern = "%" + escaped + "%";
    return RunQuery(
        "SELECT id, name, phone, email FROM customers WHERE name LIKE ? ESCAPE '\\' ORDER BY id ASC;",
        likePattern);
}

bool CustomerRepository::Update(const Customer& customer) {
    Statement stmt(db_, "UPDATE customers SET name = ?, phone = ?, email = ? WHERE id = ?;");
    sqlite3_bind_text(stmt.get(), 1, customer.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 2, customer.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 3, customer.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.get(), 4, customer.id);
    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw CustomerRepositoryError(std::string("更新に失敗しました: ") + sqlite3_errmsg(db_));
    }
    return sqlite3_changes(db_) > 0;
}

bool CustomerRepository::Remove(long long id) {
    Statement stmt(db_, "DELETE FROM customers WHERE id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, id);
    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw CustomerRepositoryError(std::string("削除に失敗しました: ") + sqlite3_errmsg(db_));
    }
    return sqlite3_changes(db_) > 0;
}

}  // namespace customer
