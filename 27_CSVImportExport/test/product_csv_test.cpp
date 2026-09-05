#include "product_csv.h"

#include <gtest/gtest.h>
#include <sqlite3.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

// テスト用の一時CSVファイルパスを生成し、デストラクタで確実に削除するRAIIヘルパー。
class TempCsvFile {
   public:
    TempCsvFile() { path_ = std::filesystem::temp_directory_path() / "product_csv_test.csv"; }
    ~TempCsvFile() { std::filesystem::remove(path_); }

    const std::string& Path() const {
        static const std::string cached = path_.string();
        return cached;
    }

   private:
    std::filesystem::path path_;
};

// テストごとに独立したインメモリDB+productsテーブルを用意するフィクスチャ。
class ProductCsvTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ASSERT_EQ(sqlite3_open(":memory:", &db_), SQLITE_OK);
        constexpr const char* kCreateTable =
            "CREATE TABLE products (id INTEGER PRIMARY KEY, name TEXT NOT NULL,"
            " price INTEGER NOT NULL, stock INTEGER NOT NULL);";
        ASSERT_EQ(sqlite3_exec(db_, kCreateTable, nullptr, nullptr, nullptr), SQLITE_OK);
    }

    void TearDown() override { sqlite3_close(db_); }

    void InsertProduct(long long id, const std::string& name, long long price, long long stock) {
        std::ostringstream sql;
        sql << "INSERT INTO products (id, name, price, stock) VALUES (" << id << ", '" << name
            << "', " << price << ", " << stock << ");";
        ASSERT_EQ(sqlite3_exec(db_, sql.str().c_str(), nullptr, nullptr, nullptr), SQLITE_OK);
    }

    static std::string ReadFile(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        std::ostringstream oss;
        oss << in.rdbuf();
        return oss.str();
    }

    sqlite3* db_ = nullptr;
};

}  // namespace

TEST_F(ProductCsvTest, ExportWritesHeaderOnlyWhenTableIsEmpty) {
    TempCsvFile file;

    product_csv::ExportProductsToCsv(db_, file.Path());

    EXPECT_EQ(ReadFile(file.Path()), "id,name,price,stock\n");
}

TEST_F(ProductCsvTest, ExportWritesAllRows) {
    InsertProduct(1, "コーヒー", 500, 10);
    InsertProduct(2, "紅茶", 400, 20);
    TempCsvFile file;

    product_csv::ExportProductsToCsv(db_, file.Path());

    EXPECT_EQ(ReadFile(file.Path()), "id,name,price,stock\n1,コーヒー,500,10\n2,紅茶,400,20\n");
}

TEST_F(ProductCsvTest, ImportInsertsValidRows) {
    TempCsvFile file;
    {
        std::ofstream out(file.Path(), std::ios::binary);
        out << "id,name,price,stock\n1,コーヒー,500,10\n2,紅茶,400,20\n";
    }

    const auto result = product_csv::ImportProductsFromCsv(db_, file.Path());

    EXPECT_EQ(result.importedCount, 2);
    EXPECT_TRUE(result.errors.empty());
}

// 列数が4でない行や、id/price/stockが整数として解釈できない行はスキップされ、
// エラーとして記録される。1行の不備で全体は失敗しない。
TEST_F(ProductCsvTest, ImportSkipsInvalidRowsAndRecordsErrors) {
    TempCsvFile file;
    {
        std::ofstream out(file.Path(), std::ios::binary);
        out << "id,name,price,stock\n"
            << "1,コーヒー,500,10\n"       // 正常
            << "2,不正,not-a-number,5\n"  // priceが非整数
            << "3,列不足,100\n";           // 列数不足(3列)
    }

    const auto result = product_csv::ImportProductsFromCsv(db_, file.Path());

    EXPECT_EQ(result.importedCount, 1);
    EXPECT_EQ(result.errors.size(), 2u);
}

// idが既存行と重複する場合は上書き(INSERT OR REPLACE)される。
TEST_F(ProductCsvTest, ImportOverwritesExistingRowWithSameId) {
    InsertProduct(1, "旧コーヒー", 300, 5);
    TempCsvFile file;
    {
        std::ofstream out(file.Path(), std::ios::binary);
        out << "id,name,price,stock\n1,新コーヒー,500,10\n";
    }

    product_csv::ImportProductsFromCsv(db_, file.Path());

    TempCsvFile exportFile;
    product_csv::ExportProductsToCsv(db_, exportFile.Path());
    EXPECT_EQ(ReadFile(exportFile.Path()), "id,name,price,stock\n1,新コーヒー,500,10\n");
}

TEST_F(ProductCsvTest, ImportOfEmptyFileDoesNothing) {
    TempCsvFile file;
    { std::ofstream out(file.Path(), std::ios::binary); }  // 空ファイル

    const auto result = product_csv::ImportProductsFromCsv(db_, file.Path());

    EXPECT_EQ(result.importedCount, 0);
    EXPECT_TRUE(result.errors.empty());
}

TEST_F(ProductCsvTest, ExportThrowsWhenFilePathIsInvalid) {
    // 存在しないディレクトリへの書き込みは失敗する。
    EXPECT_THROW(product_csv::ExportProductsToCsv(db_, "no_such_dir/out.csv"), product_csv::ProductCsvError);
}

TEST_F(ProductCsvTest, ImportThrowsWhenFileDoesNotExist) {
    EXPECT_THROW(product_csv::ImportProductsFromCsv(db_, "no_such_file_12345.csv"),
                 product_csv::ProductCsvError);
}
