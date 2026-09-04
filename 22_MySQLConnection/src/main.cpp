// 22. MySQL/PostgreSQL接続
//
// サーバー型DB(MySQLまたはPostgreSQL)にODBC経由で接続し、簡単なCRUDを行うCLI。
// 開発環境にMySQL/PostgreSQLサーバーもODBCドライバも無いため、実際に接続が
// 成功する場面は確認できていない。その代わり、
//   - 接続文字列の組み立てロジック
//   - 接続失敗時にSQLGetDiagRecベースのエラーメッセージが正しく表示される経路
// は実際に動作を確認している(詳細はREADME.mdを参照)。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>

#include "connection_string.h"
#include "odbc_connection.h"

namespace {

void PrintUsage(const char* programName) {
    std::cout << "使い方: " << programName
              << " <mysql|postgres> <host> <port> <database> <user> <password> [driver名]\n"
              << "例: " << programName << " mysql 127.0.0.1 3306 sampledb appuser apppass\n";
}

void RunCrudDemo(dbconn::OdbcConnection& conn) {
    conn.ExecuteUpdate(
        "CREATE TABLE IF NOT EXISTS demo_items ("
        "id INT PRIMARY KEY, name VARCHAR(50), price INT)");
    conn.ExecuteUpdate("DELETE FROM demo_items WHERE id = 1");
    conn.ExecuteUpdate("INSERT INTO demo_items (id, name, price) VALUES (1, 'Widget', 100)");
    conn.ExecuteUpdate("UPDATE demo_items SET price = 150 WHERE id = 1");

    const dbconn::QueryResult result = conn.ExecuteQuery("SELECT id, name, price FROM demo_items");
    std::cout << "列: ";
    for (const std::string& name : result.columnNames) {
        std::cout << name << " ";
    }
    std::cout << "\n";
    for (const std::vector<std::string>& row : result.rows) {
        for (const std::string& value : row) {
            std::cout << value << " ";
        }
        std::cout << "\n";
    }

    conn.ExecuteUpdate("DELETE FROM demo_items WHERE id = 1");
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc < 7) {
        PrintUsage(argv[0]);
        return 1;
    }

    const std::string kindArg = argv[1];
    dbconn::ConnectionParams params;
    params.kind = kindArg == "postgres" ? dbconn::ServerKind::kPostgreSql : dbconn::ServerKind::kMySql;
    params.host = argv[2];
    params.port = static_cast<uint16_t>(std::stoi(argv[3]));
    params.database = argv[4];
    params.user = argv[5];
    params.password = argv[6];
    const std::string driverOverride = argc > 7 ? argv[7] : "";

    const std::string connStr = dbconn::BuildOdbcConnectionString(params, driverOverride);
    // パスワードそのものは表示せず、組み立てロジックが正しく動いていることだけを
    // 目視確認できるよう、Pwd=以降をマスクして表示する。
    // BuildOdbcConnectionString()は常にPwdを最後のキーワードとして追加する実装のため、
    // Pwd=以降を丸ごと置き換えれば足りる(パスワード自体に';'や'{'/'}'が含まれ
    // エスケープされていても、途中で区切りを誤検出する心配がない)。
    std::string maskedConnStr = connStr;
    const size_t pwdPos = maskedConnStr.find("Pwd=");
    if (pwdPos != std::string::npos) {
        maskedConnStr.replace(pwdPos, maskedConnStr.size() - pwdPos, "Pwd=****;");
    }
    std::cout << "接続文字列: " << maskedConnStr << "\n";

    dbconn::OdbcConnection conn;
    try {
        conn.Connect(connStr);
        std::cout << "接続に成功しました。CRUDデモを実行します。\n";
        RunCrudDemo(conn);
    } catch (const dbconn::OdbcError& e) {
        std::cerr << "接続またはクエリの実行に失敗しました。\n" << e.what() << "\n";
        return 1;
    }

    return 0;
}
