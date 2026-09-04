// 37. WebSocket通信 - クライアント側
//
// WebSocketハンドシェイクを行い、複数のテキストメッセージを送って
// エコー応答を表示する。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <vector>

#include "tcp_socket.h"
#include "websocket.h"
#include "winsock_guard.h"

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    if (argc < 3) {
        std::cerr << "使い方: " << argv[0] << " <host> <port> [メッセージ...]\n";
        return 1;
    }
    const std::string host = argv[1];
    const uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    std::vector<std::string> messages;
    if (argc > 3) {
        for (int i = 3; i < argc; ++i) {
            messages.emplace_back(argv[i]);
        }
    } else {
        messages = {"Hello", "WebSocket", "こんにちは"};
    }

    try {
        net::WinsockGuard guard;
        net::TcpConnection connection = net::TcpConnection::Connect(host, port);
        ws::PerformClientHandshake(connection, host, "/");
        std::cout << "ハンドシェイクに成功しました。\n";

        for (const std::string& message : messages) {
            // クライアント→サーバーのフレームは必ずマスクする(RFC 6455)。
            ws::SendTextFrame(connection, message, /*masked=*/true);
            std::cout << "送信: " << message << "\n";
            // サーバー→クライアントのフレームは非マスクでなければならない(RFC 6455)。
            const std::string response = ws::ReceiveTextFrame(connection, /*expectMasked=*/false);
            std::cout << "  受信: " << response << "\n";
        }
    } catch (const net::TcpError& e) {
        std::cerr << "TCPエラー: " << e.what() << "\n";
        return 1;
    } catch (const ws::WebSocketError& e) {
        std::cerr << "WebSocketエラー: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
