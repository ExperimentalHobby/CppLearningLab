#include "repository.h"

#include <gtest/gtest.h>
#include <sqlite3.h>

#include "product.h"
#include "user.h"

using orm::Product;
using orm::ProductMapper;
using orm::Repository;
using orm::User;
using orm::UserMapper;

namespace {

// テストごとに独立したインメモリDBを用意するフィクスチャ。
class RepositoryTest : public ::testing::Test {
   protected:
    void SetUp() override { ASSERT_EQ(sqlite3_open(":memory:", &db_), SQLITE_OK); }
    void TearDown() override { sqlite3_close(db_); }

    sqlite3* db_ = nullptr;
};

}  // namespace

TEST_F(RepositoryTest, UserFindAllReturnsEmptyBeforeAnySave) {
    Repository<User, UserMapper> repo(db_);
    repo.EnsureSchema();

    EXPECT_TRUE(repo.FindAll().empty());
}

// idが0のエンティティをSave()するとINSERTになり、採番されたidがエンティティに反映される。
TEST_F(RepositoryTest, UserSaveInsertsNewEntityAndAssignsId) {
    Repository<User, UserMapper> repo(db_);
    repo.EnsureSchema();

    User user;
    user.name = "Alice";
    user.email = "alice@example.com";
    const long long id = repo.Save(user);

    EXPECT_GT(id, 0);
    EXPECT_EQ(user.id, id);
}

TEST_F(RepositoryTest, UserFindByIdReturnsSavedEntity) {
    Repository<User, UserMapper> repo(db_);
    repo.EnsureSchema();
    User user;
    user.name = "Alice";
    user.email = "alice@example.com";
    repo.Save(user);

    User found;
    const bool ok = repo.FindById(user.id, found);

    EXPECT_TRUE(ok);
    EXPECT_EQ(found.name, "Alice");
    EXPECT_EQ(found.email, "alice@example.com");
}

TEST_F(RepositoryTest, UserFindByIdReturnsFalseForMissingId) {
    Repository<User, UserMapper> repo(db_);
    repo.EnsureSchema();

    User found;
    EXPECT_FALSE(repo.FindById(999, found));
}

// idが0でないエンティティをSave()するとUPDATEになる。
TEST_F(RepositoryTest, UserSaveUpdatesExistingEntity) {
    Repository<User, UserMapper> repo(db_);
    repo.EnsureSchema();
    User user;
    user.name = "Alice";
    user.email = "alice@example.com";
    repo.Save(user);

    user.email = "alice-new@example.com";
    repo.Save(user);

    User found;
    repo.FindById(user.id, found);
    EXPECT_EQ(found.email, "alice-new@example.com");
}

TEST_F(RepositoryTest, UserRemoveDeletesEntity) {
    Repository<User, UserMapper> repo(db_);
    repo.EnsureSchema();
    User user;
    user.name = "Alice";
    user.email = "alice@example.com";
    repo.Save(user);

    const bool removed = repo.Remove(user.id);

    EXPECT_TRUE(removed);
    User found;
    EXPECT_FALSE(repo.FindById(user.id, found));
}

TEST_F(RepositoryTest, UserRemoveReturnsFalseForMissingId) {
    Repository<User, UserMapper> repo(db_);
    repo.EnsureSchema();

    EXPECT_FALSE(repo.Remove(999));
}

// Repository<T, Mapper>本体を変更せずに、別のエンティティ(Product)へも
// 同じ操作一式が適用できることを確認する(テンプレートの汎用性の検証)。
TEST_F(RepositoryTest, ProductSaveAndFindByIdRoundTrip) {
    Repository<Product, ProductMapper> repo(db_);
    repo.EnsureSchema();

    Product product;
    product.name = "コーヒー";
    product.price = 500;
    const long long id = repo.Save(product);

    Product found;
    ASSERT_TRUE(repo.FindById(id, found));
    EXPECT_EQ(found.name, "コーヒー");
    EXPECT_EQ(found.price, 500);
}

TEST_F(RepositoryTest, ProductFindAllReturnsAllSavedEntities) {
    Repository<Product, ProductMapper> repo(db_);
    repo.EnsureSchema();
    Product a;
    a.name = "コーヒー";
    a.price = 500;
    repo.Save(a);
    Product b;
    b.name = "紅茶";
    b.price = 400;
    repo.Save(b);

    const auto products = repo.FindAll();

    ASSERT_EQ(products.size(), 2u);
}
