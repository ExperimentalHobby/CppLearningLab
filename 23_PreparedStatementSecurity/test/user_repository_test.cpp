#include "user_repository.h"

#include <gtest/gtest.h>

using security::UserRepository;

// UserRepositoryはコピー・ムーブのいずれも不可(sqlite3*を所有し、デストラクタで
// 解放するため)なので、ヘルパー関数で生成して返すのではなく、各テストで直接
// スタック上に構築する。

TEST(UserRepositorySafeTest, FindsExistingUserByExactUsername) {
    UserRepository repo;
    repo.OpenInMemoryAndSeed();

    const auto users = repo.FindByUsernameSafe("alice");

    ASSERT_EQ(users.size(), 1u);
    EXPECT_EQ(users[0].username, "alice");
}

TEST(UserRepositorySafeTest, ReturnsEmptyForNonExistentUsername) {
    UserRepository repo;
    repo.OpenInMemoryAndSeed();

    EXPECT_TRUE(repo.FindByUsernameSafe("nobody").empty());
}

// SQLインジェクションを狙った入力を渡しても、プレースホルダはSQLの構造を
// 変えないため、単に「そのままの文字列に一致するユーザー」が無いだけになる。
TEST(UserRepositorySafeTest, TreatsInjectionPayloadAsLiteralString) {
    UserRepository repo;
    repo.OpenInMemoryAndSeed();

    const auto users = repo.FindByUsernameSafe("' OR '1'='1");

    EXPECT_TRUE(users.empty());
}

TEST(UserRepositoryUnsafeTest, FindsExistingUserByExactUsername) {
    UserRepository repo;
    repo.OpenInMemoryAndSeed();

    const auto users = repo.FindByUsernameUnsafe("alice");

    ASSERT_EQ(users.size(), 1u);
    EXPECT_EQ(users[0].username, "alice");
}

// 脆弱性の実演: 文字列連結でSQLを組み立てるUnsafe版は、WHERE句を
// 常に真にする入力を与えると、本来ヒットしないはずの全ユーザーを返してしまう。
TEST(UserRepositoryUnsafeTest, InjectionPayloadReturnsAllUsers) {
    UserRepository repo;
    repo.OpenInMemoryAndSeed();

    const auto users = repo.FindByUsernameUnsafe("' OR '1'='1");

    EXPECT_EQ(users.size(), 3u);
}
