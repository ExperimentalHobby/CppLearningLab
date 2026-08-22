// 32. UDP通信 - サーバー側
//
// 指定ポートで受信し、届いたデータグラムを送信元へそのまま送り返す
// (エコー)。TCPと違いコネクションを張らないため、"接続"や"切断"という
// 概念が無く、次にどのアドレスからデータが届くかは毎回変わりうる。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>

#include "udp_socket.h"
#include "winsock_guard.h"

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        int portInt = 12346;
        if (argc > 1) {
            try {
                portInt = std::stoi(argv[1]);
            } catch (const std::invalid_argument&) {
                throw net::UdpError("ポート番号の形式が不正です: " + std::string(argv[1]));
            } catch (const std::out_of_range&) {
                throw net::UdpError("ポート番号が範囲外です: " + std::string(argv[1]));
            }
        }
        if (portInt < 0 || portInt > 65535) {
            throw net::UdpError("ポート番号が範囲外です: " + std::to_string(portInt));
        }
        const uint16_t port = static_cast<uint16_t>(portInt);
        net::WinsockGuard guard;
        net::UdpSocket socket;
        socket.Bind(port);
        std::cout << "UDPEchoServer: ポート " << port << " で受信中... (Ctrl+Cで終了)\n";

        for (;;) {
            std::string data;
            net::Endpoint from;
            const size_t received = socket.ReceiveFrom(data, from);
            std::cout << "受信(" << received << "バイト, from " << from.address << ":" << from.port
                      << "): " << data << "\n";
            socket.SendTo(data, from);
        }
    } catch (const net::UdpError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
}
