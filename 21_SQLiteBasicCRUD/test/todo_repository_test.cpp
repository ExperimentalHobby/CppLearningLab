#include "todo_repository.h"

#include <gtest/gtest.h>

using todo::TodoItem;
using todo::TodoRepository;

namespace {

// 各テストごとに独立したインメモリDBを用意する(ファイル不要、実行後は自動的に破棄される)。
TodoRepository OpenInMemoryRepository() {
    TodoRepository repo;
    repo.Open(":memory:");
    return repo;
}

}  // namespace

TEST(TodoRepositoryTest, AddReturnsAssignedId) {
    auto repo = OpenInMemoryRepository();

    const long long id = repo.Add("牛乳を買う");

    EXPECT_GT(id, 0);
}

TEST(TodoRepositoryTest, ListReturnsItemsInIdOrder) {
    auto repo = OpenInMemoryRepository();
    repo.Add("1つ目");
    repo.Add("2つ目");

    const auto items = repo.List();

    ASSERT_EQ(items.size(), 2u);
    EXPECT_EQ(items[0].title, "1つ目");
    EXPECT_EQ(items[1].title, "2つ目");
    EXPECT_FALSE(items[0].done);
}

TEST(TodoRepositoryTest, FindByIdReturnsTrueForExistingItem) {
    auto repo = OpenInMemoryRepository();
    const long long id = repo.Add("牛乳を買う");

    TodoItem item;
    const bool found = repo.FindById(id, item);

    EXPECT_TRUE(found);
    EXPECT_EQ(item.title, "牛乳を買う");
}

TEST(TodoRepositoryTest, FindByIdReturnsFalseForMissingItem) {
    auto repo = OpenInMemoryRepository();

    TodoItem item;
    EXPECT_FALSE(repo.FindById(999, item));
}

TEST(TodoRepositoryTest, UpdateChangesTitleAndDoneState) {
    auto repo = OpenInMemoryRepository();
    const long long id = repo.Add("牛乳を買う");

    const bool updated = repo.Update(id, "パンを買う", true);

    EXPECT_TRUE(updated);
    TodoItem item;
    repo.FindById(id, item);
    EXPECT_EQ(item.title, "パンを買う");
    EXPECT_TRUE(item.done);
}

// 存在しないidへのUpdateは、SQL自体は正常終了するが対象行が無いのでfalseを返す。
TEST(TodoRepositoryTest, UpdateReturnsFalseForMissingItem) {
    auto repo = OpenInMemoryRepository();
    EXPECT_FALSE(repo.Update(999, "存在しない", false));
}

TEST(TodoRepositoryTest, SetDoneUpdatesOnlyDoneFlag) {
    auto repo = OpenInMemoryRepository();
    const long long id = repo.Add("牛乳を買う");

    const bool updated = repo.SetDone(id, true);

    EXPECT_TRUE(updated);
    TodoItem item;
    repo.FindById(id, item);
    EXPECT_EQ(item.title, "牛乳を買う");
    EXPECT_TRUE(item.done);
}

TEST(TodoRepositoryTest, RemoveDeletesExistingItem) {
    auto repo = OpenInMemoryRepository();
    const long long id = repo.Add("牛乳を買う");

    const bool removed = repo.Remove(id);

    EXPECT_TRUE(removed);
    TodoItem item;
    EXPECT_FALSE(repo.FindById(id, item));
}

TEST(TodoRepositoryTest, RemoveReturnsFalseForMissingItem) {
    auto repo = OpenInMemoryRepository();
    EXPECT_FALSE(repo.Remove(999));
}
