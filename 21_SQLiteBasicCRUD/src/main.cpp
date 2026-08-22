// 21. SQLite基本CRUD
//
// SQLiteをバックエンドにしたTODOリストを操作する対話的CLIツール。
// add/list/done/update/remove の一連のCRUD操作を体験できる。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <sstream>
#include <string>

#include "todo_repository.h"

namespace {

void PrintHelp() {
    std::cout << "コマンド一覧:\n"
              << "  add <タイトル>          新しいTODOを追加\n"
              << "  list                    一覧表示\n"
              << "  done <id>               完了にする\n"
              << "  update <id> <タイトル>  タイトルを変更\n"
              << "  remove <id>             削除\n"
              << "  help                    このヘルプを表示\n"
              << "  quit / exit             終了\n";
}

void PrintList(const todo::TodoRepository& repo) {
    const std::vector<todo::TodoItem> items = repo.List();
    if (items.empty()) {
        std::cout << "(TODOはありません)\n";
        return;
    }
    for (const todo::TodoItem& item : items) {
        std::cout << "[" << (item.done ? "x" : " ") << "] #" << item.id << " " << item.title << "\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    const std::string dbPath = argc > 1 ? argv[1] : "todo.db";

    todo::TodoRepository repo;
    try {
        repo.Open(dbPath);
    } catch (const todo::TodoRepositoryError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }

    std::cout << "21. SQLite基本CRUD - DBファイル: " << dbPath << "\n";
    PrintHelp();

    std::string line;
    while (std::cout << "> " && std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        if (command.empty()) {
            continue;
        }

        try {
            if (command == "add") {
                std::string title;
                std::getline(iss, title);
                if (!title.empty() && title.front() == ' ') {
                    title.erase(0, 1);
                }
                if (title.empty()) {
                    std::cout << "タイトルを指定してください。\n";
                    continue;
                }
                const long long id = repo.Add(title);
                std::cout << "追加しました (#" << id << ")\n";
            } else if (command == "list") {
                PrintList(repo);
            } else if (command == "done") {
                long long id = 0;
                if (!(iss >> id)) {
                    std::cout << "idを指定してください。\n";
                    continue;
                }
                std::cout << (repo.SetDone(id, true) ? "完了にしました。\n" : "該当するidがありません。\n");
            } else if (command == "update") {
                long long id = 0;
                if (!(iss >> id)) {
                    std::cout << "idを指定してください。\n";
                    continue;
                }
                std::string title;
                std::getline(iss, title);
                if (!title.empty() && title.front() == ' ') {
                    title.erase(0, 1);
                }
                if (title.empty()) {
                    std::cout << "新しいタイトルを指定してください。\n";
                    continue;
                }
                todo::TodoItem current;
                const bool exists = repo.FindById(id, current);
                const bool done = exists ? current.done : false;
                std::cout << (repo.Update(id, title, done) ? "更新しました。\n" : "該当するidがありません。\n");
            } else if (command == "remove") {
                long long id = 0;
                if (!(iss >> id)) {
                    std::cout << "idを指定してください。\n";
                    continue;
                }
                std::cout << (repo.Remove(id) ? "削除しました。\n" : "該当するidがありません。\n");
            } else if (command == "help") {
                PrintHelp();
            } else if (command == "quit" || command == "exit") {
                break;
            } else {
                std::cout << "不明なコマンドです。'help'で一覧を表示できます。\n";
            }
        } catch (const todo::TodoRepositoryError& e) {
            std::cerr << "エラー: " << e.what() << "\n";
        }
    }

    return 0;
}
