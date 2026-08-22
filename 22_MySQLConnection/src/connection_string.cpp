#include "connection_string.h"

namespace dbconn {

namespace {

// ODBC接続文字列の値に`;`や`{`/`}`が含まれる場合は`{...}`で囲む必要がある。
// 囲んだ場合、値中の`}`は`}}`に二重化してエスケープする。
std::string EscapeOdbcValue(const std::string& value) {
    const bool needsBraces =
        value.find(';') != std::string::npos || value.find('{') != std::string::npos ||
        value.find('}') != std::string::npos || value.find('=') != std::string::npos;
    if (!needsBraces) {
        return value;
    }
    std::string escaped = "{";
    for (const char c : value) {
        if (c == '}') {
            escaped += "}}";
        } else {
            escaped += c;
        }
    }
    escaped += "}";
    return escaped;
}

}  // namespace

uint16_t DefaultPort(ServerKind kind) {
    switch (kind) {
        case ServerKind::kMySql:
            return 3306;
        case ServerKind::kPostgreSql:
            return 5432;
    }
    return 0;
}

std::string DefaultDriverName(ServerKind kind) {
    switch (kind) {
        case ServerKind::kMySql:
            return "MySQL ODBC 9.0 Unicode Driver";
        case ServerKind::kPostgreSql:
            return "PostgreSQL Unicode";
    }
    return "";
}

std::string BuildOdbcConnectionString(const ConnectionParams& params,
                                       const std::string& driverNameOverride) {
    const std::string driverName =
        !driverNameOverride.empty() ? driverNameOverride : DefaultDriverName(params.kind);
    const uint16_t port = params.port != 0 ? params.port : DefaultPort(params.kind);

    std::string connStr;
    connStr += "Driver={" + driverName + "};";
    connStr += "Server=" + EscapeOdbcValue(params.host) + ";";
    connStr += "Port=" + std::to_string(port) + ";";
    connStr += "Database=" + EscapeOdbcValue(params.database) + ";";
    connStr += "Uid=" + EscapeOdbcValue(params.user) + ";";
    connStr += "Pwd=" + EscapeOdbcValue(params.password) + ";";
    return connStr;
}

}  // namespace dbconn
