// ODBC接続文字列の組み立て。
//
// MySQL/PostgreSQLはいずれも公式のODBCドライバを提供しており、ホスト名/ポート/
// DB名/認証情報を「ドライバ名+キーワード」形式のODBC接続文字列に変換するだけで
// 同じODBC APIから両方に接続できる。この変換ロジックはDBサーバー本体にも
// ODBCドライバにも依存しないため、実サーバーが無い環境でも純粋に検証できる。
#pragma once

#include <cstdint>
#include <string>

namespace dbconn {

enum class ServerKind {
    kMySql,
    kPostgreSql,
};

struct ConnectionParams {
    ServerKind kind = ServerKind::kMySql;
    std::string host;
    uint16_t port = 0;  // 0の場合は各DBの既定ポートを使う
    std::string database;
    std::string user;
    std::string password;
};

// 既定ポート(MySQL: 3306, PostgreSQL: 5432)を返す。
uint16_t DefaultPort(ServerKind kind);

// ODBCドライバ名(インストール名)を返す。
// 実際にインストールされているドライバ名は環境によって異なりうるため、
// 一般的によく使われる名称を既定値としている。
std::string DefaultDriverName(ServerKind kind);

// paramsからODBC接続文字列("Driver={...};Server=...;Port=...;...")を組み立てる。
// driverNameOverrideが空でなければ既定のドライバ名の代わりに使う。
std::string BuildOdbcConnectionString(const ConnectionParams& params,
                                       const std::string& driverNameOverride = "");

}  // namespace dbconn
