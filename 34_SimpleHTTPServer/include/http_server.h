// HttpServer: パスに応じたルーティングを行う最小限のHTTPサーバー。
//
// 接続ごとにスレッドを立てて処理する(36_MultiClientChatServerと同じ
// 「複数接続の同時処理」パターン)。1リクエスト1接続、GETのみ対応。
#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <string>

#include "http_message.h"
#include "tcp_socket.h"

namespace httpsrv {

struct RouteResponse {
    int statusCode = 200;
    std::string statusText = "OK";
    std::string contentType = "text/plain; charset=utf-8";
    std::string body;
};

using Handler = std::function<RouteResponse(const HttpRequest&)>;

class HttpServer {
   public:
    // "GET /about"のようにメソッド+パスの組にハンドラを登録する。
    void AddRoute(const std::string& method, const std::string& path, Handler handler);

    // 指定ポートで待受を開始し、1接続ごとにスレッドを立てて処理し続ける
    // (呼び出したスレッドをブロックする)。
    void Run(uint16_t port);

   private:
    // 接続ごとに無制限にスレッドを立てるとリソース枯渇(DoS)につながるため、
    // 同時処理数に上限を設ける。上限超過時は503を返して即座に接続を閉じる。
    static constexpr int kMaxConcurrentConnections = 64;

    std::map<std::string, Handler> routes_;  // キー: "GET /path"
    std::atomic<int> activeConnections_{0};

    void HandleConnection(net::TcpConnection connection);
    void RejectWithServiceUnavailable(net::TcpConnection& connection);
    RouteResponse Dispatch(const HttpRequest& request) const;
};

}  // namespace httpsrv
