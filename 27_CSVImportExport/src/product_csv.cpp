#include "product_csv.h"

#include <sqlite3.h>

#include <fstream>
#include <sstream>

#include "csv_utils.h"

namespace product_csv {

namespace {

class Statement {
   public:
    Statement(sqlite3* db, const char* sql) {
        if (sqlite3_prepare_v2(db, sql, -1, &stmt_, nullptr) != SQLITE_OK) {
            throw ProductCsvError(std::string("プリペアドステートメントの作成に失敗しました: ") +
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

// 文字列全体が(先頭の空白等を除き)1つの整数として解釈できるかを厳密に確認する。
// std::stollは末尾に余計な文字があっても例外を投げず途中まで変換してしまうため、
// "123abc"のような不正な値を誤って受理しないようにここでチェックする。
bool TryParseInt64(const std::string& text, long long& out) {
    if (text.empty()) {
        return false;
    }
    try {
        size_t consumed = 0;
        const long long value = std::stoll(text, &consumed);
        if (consumed != text.size()) {
            return false;
        }
        out = value;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

}  // namespace

void ExportProductsToCsv(sqlite3* db, const std::string& filePath) {
    Statement stmt(db, "SELECT id, name, price, stock FROM products ORDER BY id ASC;");

    std::ofstream out(filePath, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw ProductCsvError("CSVファイルを開けませんでした: " + filePath);
    }

    out << csv::BuildCsvLine({"id", "name", "price", "stock"}) << "\n";

    int rc = 0;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        const long long id = sqlite3_column_int64(stmt.get(), 0);
        const unsigned char* name = sqlite3_column_text(stmt.get(), 1);
        const long long price = sqlite3_column_int64(stmt.get(), 2);
        const long long stock = sqlite3_column_int64(stmt.get(), 3);

        csv::Row row = {std::to_string(id), name != nullptr ? reinterpret_cast<const char*>(name) : "",
                         std::to_string(price), std::to_string(stock)};
        out << csv::BuildCsvLine(row) << "\n";
    }
    if (rc != SQLITE_DONE) {
        throw ProductCsvError(std::string("エクスポート中の読み取りに失敗しました: ") + sqlite3_errmsg(db));
    }
}

ImportResult ImportProductsFromCsv(sqlite3* db, const std::string& filePath) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        throw ProductCsvError("CSVファイルを開けませんでした: " + filePath);
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    const std::vector<csv::Row> rows = csv::ParseCsv(buffer.str());

    ImportResult result;
    if (rows.empty()) {
        return result;  // 空ファイル。ヘッダーすら無いので何もしない。
    }

    Statement stmt(db, "INSERT OR REPLACE INTO products (id, name, price, stock) VALUES (?, ?, ?, ?);");

    // rows[0]はヘッダー行("id,name,price,stock")として読み飛ばす。
    for (size_t i = 1; i < rows.size(); ++i) {
        const csv::Row& row = rows[i];
        const int lineNumber = static_cast<int>(i) + 1;  // 1-origin、ヘッダー分+1

        if (row.size() != 4) {
            result.errors.push_back(std::to_string(lineNumber) + "行目: 列数が4ではありません(" +
                                     std::to_string(row.size()) + "列)");
            continue;
        }

        long long id = 0;
        long long price = 0;
        long long stock = 0;
        if (!TryParseInt64(row[0], id)) {
            result.errors.push_back(std::to_string(lineNumber) + "行目: idが整数ではありません(\"" + row[0] + "\")");
            continue;
        }
        if (!TryParseInt64(row[2], price)) {
            result.errors.push_back(std::to_string(lineNumber) + "行目: priceが整数ではありません(\"" + row[2] +
                                     "\")");
            continue;
        }
        if (!TryParseInt64(row[3], stock)) {
            result.errors.push_back(std::to_string(lineNumber) + "行目: stockが整数ではありません(\"" + row[3] +
                                     "\")");
            continue;
        }

        sqlite3_reset(stmt.get());
        sqlite3_clear_bindings(stmt.get());
        sqlite3_bind_int64(stmt.get(), 1, id);
        sqlite3_bind_text(stmt.get(), 2, row[1].c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt.get(), 3, price);
        sqlite3_bind_int64(stmt.get(), 4, stock);
        if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
            result.errors.push_back(std::to_string(lineNumber) + "行目: DB登録に失敗しました(" +
                                     sqlite3_errmsg(db) + ")");
            continue;
        }
        ++result.importedCount;
    }

    return result;
}

}  // namespace product_csv
