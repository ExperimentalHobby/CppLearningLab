// 31. TCPクライアント/サーバー - サーバー側
//
// 指定ポートで待受し、接続してきたクライアントが送ってきた行をそのまま
// 送り返す(エコー)。1クライアントとのやり取りが終わる(切断される)たびに、
// 次のクライアントの接続を待つ。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "tcp_socket.h"
#include "winsock_guard.h"

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        int parsedPort = 12345;
        if (argc > 1) {
            try {
                parsedPort = std::stoi(argv[1]);
                if (parsedPort < 0 || parsedPort > std::numeric_limits<uint16_t>::max()) {
                    throw std::out_of_range("port out of range");
                }
            } catch (const std::exception&) {
                throw std::runtime_error("ポート番号は0から65535の整数で指定してください。");
            }
        }
        const uint16_t port = static_cast<uint16_t>(parsedPort);

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
    } catch (const std::exception& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
}
