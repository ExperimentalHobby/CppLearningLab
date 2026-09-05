#include "connection_string.h"

#include <gtest/gtest.h>

using namespace dbconn;

TEST(DefaultPortTest, ReturnsWellKnownPorts) {
    EXPECT_EQ(DefaultPort(ServerKind::kMySql), 3306);
    EXPECT_EQ(DefaultPort(ServerKind::kPostgreSql), 5432);
}

TEST(DefaultDriverNameTest, ReturnsNonEmptyDriverNames) {
    EXPECT_FALSE(DefaultDriverName(ServerKind::kMySql).empty());
    EXPECT_FALSE(DefaultDriverName(ServerKind::kPostgreSql).empty());
}

TEST(BuildOdbcConnectionStringTest, UsesDefaultPortWhenNotSpecified) {
    ConnectionParams params;
    params.kind = ServerKind::kMySql;
    params.host = "localhost";
    params.database = "testdb";
    params.user = "root";
    params.password = "secret";

    const std::string connStr = BuildOdbcConnectionString(params);

    EXPECT_NE(connStr.find("Port=3306;"), std::string::npos);
    EXPECT_NE(connStr.find("Server=localhost;"), std::string::npos);
    EXPECT_NE(connStr.find("Database=testdb;"), std::string::npos);
}

TEST(BuildOdbcConnectionStringTest, UsesExplicitPortWhenSpecified) {
    ConnectionParams params;
    params.kind = ServerKind::kPostgreSql;
    params.host = "db.example.com";
    params.port = 15432;

    const std::string connStr = BuildOdbcConnectionString(params);

    EXPECT_NE(connStr.find("Port=15432;"), std::string::npos);
}

// driverNameOverrideを指定すると、既定のドライバ名の代わりにそちらが使われる。
TEST(BuildOdbcConnectionStringTest, DriverNameOverrideTakesPrecedence) {
    ConnectionParams params;
    params.kind = ServerKind::kMySql;

    const std::string connStr = BuildOdbcConnectionString(params, "My Custom Driver");

    EXPECT_NE(connStr.find("Driver={My Custom Driver};"), std::string::npos);
}

// 値に';'が含まれる場合は'{...}'で囲んでエスケープする。
TEST(BuildOdbcConnectionStringTest, EscapesValuesContainingSemicolon) {
    ConnectionParams params;
    params.kind = ServerKind::kMySql;
    params.password = "pa;ss";

    const std::string connStr = BuildOdbcConnectionString(params);

    EXPECT_NE(connStr.find("Pwd={pa;ss};"), std::string::npos);
}

// 値に'}'が含まれる場合は、囲んだ上で内部の'}'を'}}'に二重化する。
TEST(BuildOdbcConnectionStringTest, EscapesClosingBraceInsideValue) {
    ConnectionParams params;
    params.kind = ServerKind::kMySql;
    params.password = "a}b";

    const std::string connStr = BuildOdbcConnectionString(params);

    EXPECT_NE(connStr.find("Pwd={a}}b};"), std::string::npos);
}
