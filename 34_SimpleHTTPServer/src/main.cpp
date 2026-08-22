// 34. 簡易HTTPサーバー
//
// 31番のTCPサーバーの上にHTTPプロトコルを自作で実装し、パスに応じたレスポンスを
// 返す最小限のHTTPサーバー。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>

#include "http_server.h"

namespace {

constexpr const char* kTopPageHtml =
    "<!DOCTYPE html>\n"
    "<html><head><meta charset=\"utf-8\"><title>Simple HTTP Server</title></head>\n"
    "<body>\n"
    "<h1>34. 簡易HTTPサーバー</h1>\n"
    "<p>これは標準ソケットAPI(Winsock2)のみで自作したHTTPサーバーです。</p>\n"
    "<p><a href=\"/about\">/about</a> にもアクセスできます。</p>\n"
    "</body></html>\n";

constexpr const char* kAboutPageHtml =
    "<!DOCTYPE html>\n"
    "<html><head><meta charset=\"utf-8\"><title>About</title></head>\n"
    "<body>\n"
    "<p>31番のTCPサーバー実装を拡張し、HTTPリクエストのパースと\n"
    "レスポンス生成を自作しています。</p>\n"
    "</body></html>\n";

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    const uint16_t port = static_cast<uint16_t>(argc > 1 ? std::stoi(argv[1]) : 8080);

    httpsrv::HttpServer server;

    server.AddRoute("GET", "/", [](const httpsrv::HttpRequest&) {
        httpsrv::RouteResponse response;
        response.contentType = "text/html; charset=utf-8";
        response.body = kTopPageHtml;
        return response;
    });

    server.AddRoute("GET", "/about", [](const httpsrv::HttpRequest&) {
        httpsrv::RouteResponse response;
        response.contentType = "text/html; charset=utf-8";
        response.body = kAboutPageHtml;
        return response;
    });

    try {
        server.Run(port);
    } catch (const net::TcpError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
