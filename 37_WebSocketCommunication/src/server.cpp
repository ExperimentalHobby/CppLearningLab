// 37. WebSocket通信 - サーバー側
//
// 1接続を受け付けてWebSocketハンドシェイクを行い、受信したテキスト
// メッセージに"echo: "を付けて送り返すループを行う。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>

#include "tcp_socket.h"
#include "websocket.h"
#include "winsock_guard.h"

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    const uint16_t port = static_cast<uint16_t>(argc > 1 ? std::stoi(argv[1]) : 12348);

    try {
        net::WinsockGuard guard;
        net::TcpListener listener;
        listener.Listen(port);
        std::cout << "WebSocketServer: ポート " << port << " で待受中...\n";

        for (;;) {
            net::TcpConnection connection = listener.Accept();
            std::cout << "接続を受け付けました。ハンドシェイクを行います...\n";
            try {
                ws::PerformServerHandshake(connection);
                std::cout << "ハンドシェイクに成功しました。\n";

                for (;;) {
                    const std::string message = ws::ReceiveTextFrame(connection);
                    std::cout << "受信: " << message << "\n";
                    // サーバー→クライアントのフレームはマスクしてはならない(RFC 6455)。
                    ws::SendTextFrame(connection, "echo: " + message, /*masked=*/false);
                }
            } catch (const net::TcpError&) {
                std::cout << "クライアントが切断しました。\n";
            } catch (const ws::WebSocketError& e) {
                std::cout << "WebSocketエラー: " << e.what() << "\n";
            }
        }
    } catch (const net::TcpError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
}
