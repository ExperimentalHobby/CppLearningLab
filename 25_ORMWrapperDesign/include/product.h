// Product エンティティと、Repository<Product, ProductMapper>が要求する
// インターフェースを実装するProductMapper。
//
// UserMapperとほぼ同じ形をしているが、列の型(price は整数)や列数が異なる。
// Repository<T, Mapper>本体は一切変更せずに別のエンティティへ対応できることを
// 示すのがこの2つ目のMapperの狙い。
#pragma once

#include <sqlite3.h>

#include <cstdint>
#include <string>

namespace orm {

struct Product {
    long long id = 0;
    std::string name;
    int64_t price = 0;
};

struct ProductMapper {
    static std::string TableName() { return "products"; }

    static std::string CreateTableSql() {
        return "CREATE TABLE IF NOT EXISTS products ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "name TEXT NOT NULL,"
               "price INTEGER NOT NULL"
               ");";
    }

    static std::string SelectColumnsSql() { return "name, price"; }
    static int ColumnCount() { return 2; }

    static void BindInsertParams(sqlite3_stmt* stmt, const Product& product) {
        sqlite3_bind_text(stmt, 1, product.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, product.price);
    }

    static void BindUpdateParams(sqlite3_stmt* stmt, const Product& product) {
        sqlite3_bind_text(stmt, 1, product.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, product.price);
        sqlite3_bind_int64(stmt, 3, product.id);
    }

    static Product FromStatement(sqlite3_stmt* stmt) {
        Product product;
        product.id = sqlite3_column_int64(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        product.name = name != nullptr ? reinterpret_cast<const char*>(name) : "";
        product.price = sqlite3_column_int64(stmt, 2);
        return product;
    }

    static long long GetId(const Product& product) { return product.id; }
    static void SetId(Product& product, long long id) { product.id = id; }
};

}  // namespace orm
