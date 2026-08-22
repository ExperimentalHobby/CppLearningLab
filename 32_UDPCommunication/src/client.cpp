// 32. UDP通信 - クライアント側
//
// 複数のメッセージを連続して送信し、それぞれの応答を待つ。TCPと違い
// `connect`は行わず、`sendto`のたびに宛先を指定する(コネクションレス)。
// 応答は2秒待っても届かなければタイムアウトとして扱う(31番のTCP版には
// 無い「応答が返ってこないかもしれない」という発想がUDPでは必須になる)。
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
    if (argc < 4) {
        std::cerr << "使い方: " << argv[0] << " <host> <port> <送信回数> [メッセージ接頭辞]\n";
        return 1;
    }
    try {
        int portInt = 0;
        int count = 0;
        try {
            portInt = std::stoi(argv[2]);
            count = std::stoi(argv[3]);
        } catch (const std::invalid_argument&) {
            throw net::UdpError("数値引数の形式が不正です");
        } catch (const std::out_of_range&) {
            throw net::UdpError("数値引数が範囲外です");
        }
        if (portInt < 0 || portInt > 65535) {
            throw net::UdpError("ポート番号が範囲外です: " + std::to_string(portInt));
        }
        if (count <= 0) {
            throw net::UdpError("送信回数は1以上で指定してください: " + std::to_string(count));
        }
        const net::Endpoint server{argv[1], static_cast<uint16_t>(portInt)};
        const std::string prefix = argc > 4 ? argv[4] : "msg";

        net::WinsockGuard guard;
        net::UdpSocket socket;
        socket.SetReceiveTimeout(2000);

        for (int i = 1; i <= count; ++i) {
            const std::string message = prefix + "-" + std::to_string(i);
            socket.SendTo(message, server);
            std::cout << "送信: " << message << "\n";

            try {
                std::string response;
                net::Endpoint from;
                socket.ReceiveFrom(response, from);
                std::cout << "  応答: " << response << "\n";
            } catch (const net::UdpTimeoutError& e) {
                std::cout << "  " << e.what() << "\n";
            }
        }
    } catch (const net::UdpError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
