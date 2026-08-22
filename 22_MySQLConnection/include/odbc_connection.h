// OdbcConnection: ODBC (Windows SDK標準、追加インストール不要)経由で
// サーバー型DB(MySQL/PostgreSQL等、対応するODBCドライバがあれば何でも)に
// 接続するための薄いRAIIラッパー。
//
// SQLHENV/SQLHDBCの確保・解放をコンストラクタ/デストラクタで管理し、
// エラーはSQLGetDiagRecで取得したメッセージを添えて例外として投げる。
#pragma once

// sql.h/sqlext.hはDWORD/LPWSTR等のWin32基本型が既に定義されていることを前提に
// しており、windows.hより先にインクルードするとコンパイルエラーになる。
#include <windows.h>

#include <sql.h>
#include <sqlext.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace dbconn {

class OdbcError : public std::runtime_error {
   public:
    explicit OdbcError(const std::string& message) : std::runtime_error(message) {}
};

struct QueryResult {
    std::vector<std::string> columnNames;
    std::vector<std::vector<std::string>> rows;
};

class OdbcConnection {
   public:
    OdbcConnection();
    ~OdbcConnection();

    OdbcConnection(const OdbcConnection&) = delete;
    OdbcConnection& operator=(const OdbcConnection&) = delete;

    // 接続文字列(connection_string.hで組み立てたもの)を使って接続する。
    // 失敗時はOdbcErrorを投げる(メッセージにSQLSTATE/ネイティブエラー/
    // ドライバのエラーメッセージを整形して含む)。
    void Connect(const std::string& connectionString);

    void Disconnect();

    bool IsConnected() const { return connected_; }

    // INSERT/UPDATE/DELETE/CREATE TABLE等、結果セットを返さない文を実行する。
    void ExecuteUpdate(const std::string& sql);

    // SELECT文を実行し、列名と全行を取得する。
    QueryResult ExecuteQuery(const std::string& sql);

   private:
    SQLHENV henv_ = SQL_NULL_HANDLE;
    SQLHDBC hdbc_ = SQL_NULL_HANDLE;
    bool connected_ = false;

    [[noreturn]] void ThrowError(const std::string& context, SQLSMALLINT handleType,
                                  SQLHANDLE handle) const;
};

}  // namespace dbconn
