// 27. CSVインポート/エクスポート
//
// products(id, name, price, stock)テーブルを題材に、DB→CSVエクスポートと
// CSV→DBインポートの往復、および不正な行を含むCSVの部分的な取り込みを実演する。
#ifdef _WIN32
#include <windows.h>
#endif

#include <sqlite3.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "product_csv.h"

namespace {

constexpr const char* kCsvPath = "products_export.csv";
constexpr const char* kBadCsvPath = "products_with_errors.csv";

void Exec(sqlite3* db, const char* sql) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string message = errMsg != nullptr ? errMsg : "";
        sqlite3_free(errMsg);
        throw product_csv::ProductCsvError(message);
    }
}

void PrintProductsTable(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, "SELECT id, name, price, stock FROM products ORDER BY id ASC;", -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const long long id = sqlite3_column_int64(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const long long price = sqlite3_column_int64(stmt, 2);
        const long long stock = sqlite3_column_int64(stmt, 3);
        std::cout << "  #" << id << " " << (name != nullptr ? reinterpret_cast<const char*>(name) : "")
                  << " price=" << price << " stock=" << stock << "\n";
    }
    sqlite3_finalize(stmt);
}

void PrintFileContent(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    std::cout << buffer.str();
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "DBを開けませんでした\n";
        return 1;
    }

    std::cout << "27. CSVインポート/エクスポート\n\n";

    try {
        Exec(db,
             "CREATE TABLE products (id INTEGER PRIMARY KEY, name TEXT NOT NULL, price INTEGER NOT NULL, "
             "stock INTEGER NOT NULL);");
        // カンマやダブルクォートを含む名前をわざと混ぜて、クォート処理を確認する。
        Exec(db,
             "INSERT INTO products (id, name, price, stock) VALUES"
             " (1, 'Widget, Deluxe', 100, 20),"
             " (2, 'Gadget \"Pro\"', 250, 5),"
             " (3, 'Simple Item', 50, 100);");

        std::cout << "--- エクスポート前のDB内容 ---\n";
        PrintProductsTable(db);

        product_csv::ExportProductsToCsv(db, kCsvPath);
        std::cout << "\n--- エクスポートされたCSV(" << kCsvPath << ") ---\n";
        PrintFileContent(kCsvPath);

        std::cout << "\n--- テーブルをクリアしてCSVから再インポート ---\n";
        Exec(db, "DELETE FROM products;");
        const product_csv::ImportResult roundTrip = product_csv::ImportProductsFromCsv(db, kCsvPath);
        std::cout << "インポート件数: " << roundTrip.importedCount << ", エラー件数: " << roundTrip.errors.size()
                  << "\n";
        std::cout << "インポート後のDB内容(エクスポート前と一致するはず):\n";
        PrintProductsTable(db);

        std::cout << "\n--- 不正な行を含むCSVのインポート ---\n";
        {
            std::ofstream badCsv(kBadCsvPath, std::ios::binary | std::ios::trunc);
            badCsv << "id,name,price,stock\n";
            badCsv << "10,Valid Item,300,15\n";
            badCsv << "11,Broken Price,not-a-number,10\n";  // priceが不正
            badCsv << "12,Too,Many,Columns,Here\n";          // 列数が不正
            badCsv << "13,Another Valid Item,80,40\n";
        }
        const product_csv::ImportResult badResult = product_csv::ImportProductsFromCsv(db, kBadCsvPath);
        std::cout << "インポート件数: " << badResult.importedCount << ", エラー件数: " << badResult.errors.size()
                  << "\n";
        for (const std::string& error : badResult.errors) {
            std::cout << "  エラー: " << error << "\n";
        }
        std::cout << "インポート後のDB内容(不正行はスキップされ、正常行だけ追加されているはず):\n";
        PrintProductsTable(db);
    } catch (const product_csv::ProductCsvError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        sqlite3_close(db);
        return 1;
    }

    sqlite3_close(db);
    return 0;
}
