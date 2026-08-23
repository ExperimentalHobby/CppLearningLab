#include "http_server.h"

#include <iostream>
#include <thread>

#include "winsock_guard.h"

namespace httpsrv {

void HttpServer::AddRoute(const std::string& method, const std::string& path, Handler handler) {
    routes_[method + " " + path] = std::move(handler);
}

RouteResponse HttpServer::Dispatch(const HttpRequest& request) const {
    const auto it = routes_.find(request.method + " " + request.path);
    if (it != routes_.end()) {
        return it->second(request);
    }
    RouteResponse notFound;
    notFound.statusCode = 404;
    notFound.statusText = "Not Found";
    notFound.contentType = "text/plain; charset=utf-8";
    notFound.body = "404 Not Found: " + request.method + " " + request.path;
    return notFound;
}

void HttpServer::RejectWithServiceUnavailable(net::TcpConnection& connection) {
    try {
        const std::string raw = BuildHttpResponse(503, "Service Unavailable", "text/plain; charset=utf-8",
                                                   "サーバーが混雑しています。しばらくしてから再度お試しください。");
        connection.Send(raw);
    } catch (const std::exception&) {
        // 送信失敗(相手が既に切断済み等)は無視して構わない。
    }
}

void HttpServer::HandleConnection(net::TcpConnection connection) {
    // 関数の終了経路(正常終了/早期return/例外)に関わらず必ずデクリメントする。
    struct ActiveConnectionGuard {
        std::atomic<int>& counter;
        ~ActiveConnectionGuard() { --counter; }
    } guard{activeConnections_};

    try {
        HttpRequest request;
        if (!ReadHttpRequest(connection, request)) {
            return;  // リクエストラインが読めないまま切断された
        }

        std::cout << "リクエスト: " << request.method << " " << request.path << "\n";

        const RouteResponse response = Dispatch(request);
        const std::string raw = BuildHttpResponse(response.statusCode, response.statusText, response.contentType,
                                                   response.body);
        connection.Send(raw);
        // connectionのデストラクタでcloseされる(Connection: closeレスポンスヘッダーと
        // 整合させ、1リクエスト1接続の単純なモデルにしている)。
    } catch (const std::exception& e) {
        std::cerr << "接続処理中にエラーが発生しました: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "接続処理中に不明なエラーが発生しました。\n";
    }
}

void HttpServer::Run(uint16_t port) {
    net::WinsockGuard guard;
    net::TcpListener listener;
    listener.Listen(port);
    std::cout << "SimpleHTTPServer: http://127.0.0.1:" << port << " で待受中...\n";

    for (;;) {
        net::TcpConnection connection = listener.Accept();
        if (activeConnections_.load() >= kMaxConcurrentConnections) {
            std::cerr << "同時接続数の上限(" << kMaxConcurrentConnections << ")に達したため接続を拒否しました。\n";
            RejectWithServiceUnavailable(connection);
            continue;  // connectionはここでスコープを抜けてクローズされる
        }
        ++activeConnections_;
        std::thread(&HttpServer::HandleConnection, this, std::move(connection)).detach();
    }
}

}  // namespace httpsrv
