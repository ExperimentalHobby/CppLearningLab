#include "odbc_connection.h"

#include <iterator>
#include <sstream>
#include <vector>

namespace dbconn {

namespace {

// ODBCのUnicode(W)系APIは内部的にUTF-16(SQLWCHAR=wchar_t)でやり取りする。
// ドライバマネージャ自身が生成するエラーメッセージ(例: ドライバが見つからない)は
// システムのAPIが返すものであり、A系(ANSI)APIで取得すると環境のコードページ
// (日本語環境ではShift-JIS)で返ってくるため、UTF-8前提の本プログラムの出力と
// 混ざると文字化けする。W系APIで取得しUTF-8に変換することでこれを避ける。
std::string WideToUtf8(const wchar_t* wide, int wideLength = -1) {
    if (wide == nullptr) {
        return {};
    }
    const int utf8Length = WideCharToMultiByte(CP_UTF8, 0, wide, wideLength, nullptr, 0, nullptr, nullptr);
    if (utf8Length <= 0) {
        return {};
    }
    std::string utf8(static_cast<size_t>(utf8Length), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, wideLength, utf8.data(), utf8Length, nullptr, nullptr);
    if (wideLength < 0 && !utf8.empty() && utf8.back() == '\0') {
        utf8.pop_back();  // 終端NUL変換分を取り除く
    }
    return utf8;
}

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return {};
    }
    const int wideLength =
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(static_cast<size_t>(wideLength), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), wide.data(), wideLength);
    return wide;
}

// SQLGetDiagRecWで取得できる全ての診断レコードを整形して1つの文字列にする。
// 診断レコードは複数積まれることがある(例: ドライバが見つからない場合、
// ドライバマネージャ自身のエラーに加えて詳細情報が別レコードで付くことがある)ため、
// 全件を連結して表示する。
std::string FormatDiagnostics(SQLSMALLINT handleType, SQLHANDLE handle) {
    std::ostringstream oss;
    SQLSMALLINT recordNumber = 1;
    wchar_t sqlState[6]{};
    SQLINTEGER nativeError = 0;
    wchar_t messageText[SQL_MAX_MESSAGE_LENGTH]{};
    SQLSMALLINT textLength = 0;

    bool any = false;
    while (SQLGetDiagRecW(handleType, handle, recordNumber, sqlState, &nativeError, messageText,
                           static_cast<SQLSMALLINT>(std::size(messageText)), &textLength) == SQL_SUCCESS) {
        if (any) {
            oss << " | ";
        }
        oss << "SQLSTATE=" << WideToUtf8(sqlState) << " NativeError=" << nativeError
            << " Message=" << WideToUtf8(messageText);
        any = true;
        ++recordNumber;
    }
    if (!any) {
        oss << "(診断情報なし)";
    }
    return oss.str();
}

}  // namespace

OdbcConnection::OdbcConnection() {
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv_) != SQL_SUCCESS) {
        throw OdbcError("ODBC環境ハンドルの確保に失敗しました");
    }
    SQLSetEnvAttr(henv_, SQL_ATTR_ODBC_VERSION, reinterpret_cast<void*>(SQL_OV_ODBC3), 0);

    if (SQLAllocHandle(SQL_HANDLE_DBC, henv_, &hdbc_) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv_);
        henv_ = SQL_NULL_HANDLE;
        throw OdbcError("ODBC接続ハンドルの確保に失敗しました");
    }
}

OdbcConnection::~OdbcConnection() {
    Disconnect();
    if (hdbc_ != SQL_NULL_HANDLE) {
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc_);
    }
    if (henv_ != SQL_NULL_HANDLE) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv_);
    }
}

void OdbcConnection::ThrowError(const std::string& context, SQLSMALLINT handleType,
                                 SQLHANDLE handle) const {
    throw OdbcError(context + ": " + FormatDiagnostics(handleType, handle));
}

void OdbcConnection::Connect(const std::string& connectionString) {
    const std::wstring wideConnStr = Utf8ToWide(connectionString);
    wchar_t outConnStr[1024]{};
    SQLSMALLINT outConnStrLen = 0;
    const SQLRETURN rc = SQLDriverConnectW(
        hdbc_, nullptr, const_cast<SQLWCHAR*>(reinterpret_cast<const SQLWCHAR*>(wideConnStr.c_str())),
        static_cast<SQLSMALLINT>(wideConnStr.size()), reinterpret_cast<SQLWCHAR*>(outConnStr),
        static_cast<SQLSMALLINT>(std::size(outConnStr)), &outConnStrLen, SQL_DRIVER_NOPROMPT);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        ThrowError("DB接続に失敗しました", SQL_HANDLE_DBC, hdbc_);
    }
    connected_ = true;
}

void OdbcConnection::Disconnect() {
    if (connected_) {
        SQLDisconnect(hdbc_);
        connected_ = false;
    }
}

void OdbcConnection::ExecuteUpdate(const std::string& sql) {
    const std::wstring wideSql = Utf8ToWide(sql);
    SQLHSTMT hstmt = SQL_NULL_HANDLE;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hdbc_, &hstmt) != SQL_SUCCESS) {
        ThrowError("ステートメントハンドルの確保に失敗しました", SQL_HANDLE_DBC, hdbc_);
    }
    const SQLRETURN rc = SQLExecDirectW(
        hstmt, const_cast<SQLWCHAR*>(reinterpret_cast<const SQLWCHAR*>(wideSql.c_str())), SQL_NTS);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        const std::string diag = FormatDiagnostics(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        throw OdbcError("クエリの実行に失敗しました: " + diag);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

QueryResult OdbcConnection::ExecuteQuery(const std::string& sql) {
    const std::wstring wideSql = Utf8ToWide(sql);
    SQLHSTMT hstmt = SQL_NULL_HANDLE;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hdbc_, &hstmt) != SQL_SUCCESS) {
        ThrowError("ステートメントハンドルの確保に失敗しました", SQL_HANDLE_DBC, hdbc_);
    }

    QueryResult result;
    try {
        const SQLRETURN execRc = SQLExecDirectW(
            hstmt, const_cast<SQLWCHAR*>(reinterpret_cast<const SQLWCHAR*>(wideSql.c_str())), SQL_NTS);
        if (execRc != SQL_SUCCESS && execRc != SQL_SUCCESS_WITH_INFO) {
            throw OdbcError("クエリの実行に失敗しました: " + FormatDiagnostics(SQL_HANDLE_STMT, hstmt));
        }

        SQLSMALLINT columnCount = 0;
        SQLNumResultCols(hstmt, &columnCount);
        for (SQLSMALLINT col = 1; col <= columnCount; ++col) {
            wchar_t columnName[256]{};
            SQLSMALLINT nameLength = 0;
            SQLDescribeColW(hstmt, col, reinterpret_cast<SQLWCHAR*>(columnName),
                             static_cast<SQLSMALLINT>(std::size(columnName)), &nameLength, nullptr, nullptr,
                             nullptr, nullptr);
            result.columnNames.push_back(WideToUtf8(columnName));
        }

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::vector<std::string> row;
            for (SQLSMALLINT col = 1; col <= columnCount; ++col) {
                wchar_t buffer[1024]{};
                SQLLEN indicator = 0;
                SQLGetData(hstmt, col, SQL_C_WCHAR, buffer, sizeof(buffer), &indicator);
                row.push_back(indicator == SQL_NULL_DATA ? "" : WideToUtf8(buffer));
            }
            result.rows.push_back(std::move(row));
        }
    } catch (...) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        throw;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return result;
}

}  // namespace dbconn
