// 31. TCPクライアント/サーバー - サーバー側
//
// 指定ポートで待受し、接続してきたクライアントが送ってきた行をそのまま
// 送り返す(エコー)。1クライアントとのやり取りが終わる(切断される)たびに、
// 次のクライアントの接続を待つ。
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
    const uint16_t port = static_cast<uint16_t>(argc > 1 ? std::stoi(argv[1]) : 12345);

    try {
        net::WinsockGuard guard;
        net::TcpListener listener;
        listener.Listen(port);
        std::cout << "TCPEchoServer: ポート " << port << " で待受中...\n";

        for (;;) {
            net::TcpConnection client = listener.Accept();
            std::cout << "クライアントが接続しました。\n";

            std::string line;
            while (client.ReceiveLine(line)) {
                std::cout << "受信: " << line << "\n";
                client.SendLine(line);
            }
            std::cout << "クライアントが切断しました。\n";
        }
    } catch (const net::TcpError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
}
