// TodoRepository: SQLiteをバックエンドにしたTODOリストのCRUD操作。
//
// sqlite3*の接続をRAIIで管理し、生SQL文字列連結ではなく
// sqlite3_prepare_v2 + バインドAPIで全てのクエリを実行する
// (23_PreparedStatementSecurityで扱うSQLインジェクション対策の実践でもある)。
#pragma once

#include <stdexcept>
#include <string>
#include <vector>

struct sqlite3;

namespace todo {

// DB操作全般で共通の例外型。
class TodoRepositoryError : public std::runtime_error {
   public:
    explicit TodoRepositoryError(const std::string& message) : std::runtime_error(message) {}
};

struct TodoItem {
    long long id = 0;
    std::string title;
    bool done = false;
};

class TodoRepository {
   public:
    TodoRepository() = default;
    ~TodoRepository();

    // コピー不可(sqlite3*を所有するため)、ムーブは可。
    TodoRepository(const TodoRepository&) = delete;
    TodoRepository& operator=(const TodoRepository&) = delete;
    TodoRepository(TodoRepository&& other) noexcept;
    TodoRepository& operator=(TodoRepository&& other) noexcept;

    // DBファイルを開き(無ければ作成)、todosテーブルが無ければ作成する。
    // 失敗時はTodoRepositoryErrorを投げる。
    void Open(const std::string& dbPath);

    void Close();

    // 新しいTODOを追加し、採番されたidを返す。
    long long Add(const std::string& title);

    // 全件を id 昇順で取得する。
    std::vector<TodoItem> List() const;

    // idで1件取得する。見つからなければfalseを返す。
    bool FindById(long long id, TodoItem& out) const;

    // タイトルと完了状態を更新する。該当行が無ければfalseを返す。
    bool Update(long long id, const std::string& title, bool done);

    // 完了状態のみを更新する。該当行が無ければfalseを返す。
    bool SetDone(long long id, bool done);

    // 削除する。該当行が無ければfalseを返す。
    bool Remove(long long id);

   private:
    sqlite3* db_ = nullptr;

    void EnsureSchema();
    [[noreturn]] void ThrowLastError(const std::string& context) const;
};

}  // namespace todo
