#include "customer_repository.h"

#include <gtest/gtest.h>

using customer::Customer;
using customer::CustomerRepository;

// CustomerRepositoryはコピー・ムーブのいずれも不可(sqlite3*を所有するため)なので、
// 各テストで直接スタック上に構築する。

TEST(CustomerRepositoryTest, AddReturnsAssignedId) {
    CustomerRepository repo;
    repo.Open(":memory:");

    const long long id = repo.Add({0, "山田太郎", "090-1234-5678", "yamada@example.com"});

    EXPECT_GT(id, 0);
}

TEST(CustomerRepositoryTest, ListReturnsAllCustomersInIdOrder) {
    CustomerRepository repo;
    repo.Open(":memory:");
    repo.Add({0, "山田太郎", "", ""});
    repo.Add({0, "鈴木花子", "", ""});

    const auto customers = repo.List();

    ASSERT_EQ(customers.size(), 2u);
    EXPECT_EQ(customers[0].name, "山田太郎");
    EXPECT_EQ(customers[1].name, "鈴木花子");
}

TEST(CustomerRepositoryTest, SearchFindsPartialMatch) {
    CustomerRepository repo;
    repo.Open(":memory:");
    repo.Add({0, "山田太郎", "", ""});
    repo.Add({0, "鈴木花子", "", ""});

    const auto customers = repo.Search("山田");

    ASSERT_EQ(customers.size(), 1u);
    EXPECT_EQ(customers[0].name, "山田太郎");
}

// 空文字列で検索すると全件を返す(Listと同じ結果)。
TEST(CustomerRepositoryTest, SearchWithEmptyQueryReturnsAllCustomers) {
    CustomerRepository repo;
    repo.Open(":memory:");
    repo.Add({0, "山田太郎", "", ""});
    repo.Add({0, "鈴木花子", "", ""});

    EXPECT_EQ(repo.Search("").size(), 2u);
}

// LIKEのワイルドカード文字('%'/'_')が検索文字列に含まれていても、
// エスケープ処理により文字通りの意味として扱われる(ワイルドカードとして展開されない)。
TEST(CustomerRepositoryTest, SearchEscapesLikeWildcardCharacters) {
    CustomerRepository repo;
    repo.Open(":memory:");
    repo.Add({0, "100%満足堂", "", ""});
    repo.Add({0, "山田太郎", "", ""});

    const auto customers = repo.Search("100%満足");

    ASSERT_EQ(customers.size(), 1u);
    EXPECT_EQ(customers[0].name, "100%満足堂");
}

TEST(CustomerRepositoryTest, UpdateChangesFields) {
    CustomerRepository repo;
    repo.Open(":memory:");
    const long long id = repo.Add({0, "山田太郎", "090-0000-0000", "old@example.com"});

    const bool updated = repo.Update({id, "山田太郎", "090-9999-9999", "new@example.com"});

    EXPECT_TRUE(updated);
    const auto customers = repo.List();
    ASSERT_EQ(customers.size(), 1u);
    EXPECT_EQ(customers[0].email, "new@example.com");
}

TEST(CustomerRepositoryTest, UpdateReturnsFalseForMissingId) {
    CustomerRepository repo;
    repo.Open(":memory:");

    EXPECT_FALSE(repo.Update({999, "存在しない", "", ""}));
}

TEST(CustomerRepositoryTest, RemoveDeletesExistingCustomer) {
    CustomerRepository repo;
    repo.Open(":memory:");
    const long long id = repo.Add({0, "山田太郎", "", ""});

    EXPECT_TRUE(repo.Remove(id));
    EXPECT_TRUE(repo.List().empty());
}

TEST(CustomerRepositoryTest, RemoveReturnsFalseForMissingId) {
    CustomerRepository repo;
    repo.Open(":memory:");

    EXPECT_FALSE(repo.Remove(999));
}
