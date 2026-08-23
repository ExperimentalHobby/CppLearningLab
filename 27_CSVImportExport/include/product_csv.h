// products(id, name, price, stock)テーブルとCSVファイルの相互変換。
// CSVパース自体はcsv_utils.h(DB非依存)に任せ、ここではSQLiteとの橋渡しだけを行う。
#pragma once

#include <stdexcept>
#include <string>
#include <vector>

struct sqlite3;

namespace product_csv {

class ProductCsvError : public std::runtime_error {
   public:
    explicit ProductCsvError(const std::string& message) : std::runtime_error(message) {}
};

struct ImportResult {
    int importedCount = 0;
    // 1件ごとに「行番号: 理由」の形式でエラー内容を積む。エラーがあっても
    // インポート処理全体は継続する(1行の不備で全体を失敗させない設計)。
    std::vector<std::string> errors;
};

// productsテーブルの全件を、ヘッダー行("id,name,price,stock")付きのCSVとして
// filePathに書き出す。
void ExportProductsToCsv(sqlite3* db, const std::string& filePath);

// filePathのCSVを読み込み、productsテーブルにインポートする。
// 1行目はヘッダーとして読み飛ばす。id/price/stockが整数として解釈できない行は
// スキップし、その行番号と理由をImportResult::errorsに記録する。
// idが既存行と重複する場合は上書き(INSERT OR REPLACE)する。
ImportResult ImportProductsFromCsv(sqlite3* db, const std::string& filePath);

}  // namespace product_csv
