// 31. TCPクライアント/サーバー - クライアント側
//
// サーバーに接続し、コマンドライン引数の文字列を送信、エコーされた内容を
// 表示して終了する。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>

#include "tcp_socket.h"
#include "winsock_guard.h"

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (argc < 4) {
        std::cerr << "使い方: " << argv[0] << " <host> <port> <メッセージ>\n";
        return 1;
    }
    const std::string host = argv[1];
    const uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));
    const std::string message = argv[3];

    try {
        net::WinsockGuard guard;
        net::TcpConnection connection = net::TcpConnection::Connect(host, port);
        connection.SendLine(message);

        std::string response;
        if (connection.ReceiveLine(response)) {
            std::cout << "サーバーからのエコー: " << response << "\n";
        } else {
            std::cerr << "サーバーが応答せずに切断しました。\n";
            return 1;
        }
    } catch (const net::TcpError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
