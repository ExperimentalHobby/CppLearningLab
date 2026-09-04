// HTTPリクエストのパースとレスポンスの組み立て。
//
// 標準ソケットAPI(Winsock2)のみで自作する。GETのみを対象とし、
// リクエストボディ(POST等のペイロード)は扱わない(その制約はREADMEに明記)。
// Keep-Alive等の永続的接続にも対応せず、1リクエスト1接続とする。
#pragma once

#include <map>
#include <string>

#include "tcp_socket.h"

namespace httpsrv {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;  // キーは小文字に正規化
};

// connectionから1件分のHTTPリクエスト(リクエストライン+ヘッダー)を読み取る。
// 相手が接続を閉じるなどして読み取れなかった場合はfalseを返す。
bool ReadHttpRequest(net::TcpConnection& connection, HttpRequest& outRequest);

// ステータスライン+Content-Type/Content-Lengthヘッダー+ボディを組み立てる。
std::string BuildHttpResponse(int statusCode, const std::string& statusText, const std::string& contentType,
                               const std::string& body);

}  // namespace httpsrv
