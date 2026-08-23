// 25. 簡易ORMラッパー設計
//
// Repository<T, Mapper>という1つのテンプレートクラスを使い、生SQLを一切
// 書かずにUser/Productという2種類の異なるエンティティのCRUDを行う。
#ifdef _WIN32
#include <windows.h>
#endif

#include <sqlite3.h>

#include <iostream>
#include <vector>

#include "product.h"
#include "repository.h"
#include "user.h"

namespace {

void PrintUsers(const std::vector<orm::User>& users) {
    for (const orm::User& user : users) {
        std::cout << "  #" << user.id << " " << user.name << " <" << user.email << ">\n";
    }
}

void PrintProducts(const std::vector<orm::Product>& products) {
    for (const orm::Product& product : products) {
        std::cout << "  #" << product.id << " " << product.name << " " << product.price << "円\n";
    }
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

    std::cout << "25. 簡易ORMラッパー設計\n\n";

    try {
        orm::Repository<orm::User, orm::UserMapper> users(db);
        users.EnsureSchema();

        orm::Repository<orm::Product, orm::ProductMapper> products(db);
        products.EnsureSchema();

        std::cout << "--- User: 追加 ---\n";
        orm::User alice{0, "Alice", "alice@example.com"};
        orm::User bob{0, "Bob", "bob@example.com"};
        users.Save(alice);  // idが0なのでINSERTされ、alice.idに採番結果が書き戻る
        users.Save(bob);
        std::cout << "追加後のid: alice=#" << alice.id << " bob=#" << bob.id << "\n";
        PrintUsers(users.FindAll());

        std::cout << "\n--- User: 更新 ---\n";
        alice.email = "alice.new@example.com";
        users.Save(alice);  // idが非0なのでUPDATEされる
        PrintUsers(users.FindAll());

        std::cout << "\n--- User: 削除 ---\n";
        users.Remove(bob.id);
        PrintUsers(users.FindAll());

        std::cout << "\n--- Product: 追加・一覧 ---\n";
        orm::Product widget{0, "Widget", 100};
        orm::Product gadget{0, "Gadget", 250};
        products.Save(widget);
        products.Save(gadget);
        PrintProducts(products.FindAll());

        std::cout << "\n--- Product: 検索 ---\n";
        orm::Product found;
        if (products.FindById(widget.id, found)) {
            std::cout << "  見つかった: #" << found.id << " " << found.name << " " << found.price << "円\n";
        }

        std::cout << "\nRepository<T, Mapper>という同一のテンプレートクラスで、User/Productという\n"
                     "異なるテーブル・エンティティのCRUDを、生SQLを書かずに扱えることを確認できた。\n";
    } catch (const orm::RepositoryError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        sqlite3_close(db);
        return 1;
    }

    sqlite3_close(db);
    return 0;
}
