// 36. マルチクライアントチャットサーバー - サーバー側
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>

#include "chat_server.h"
#include "tcp_socket.h"

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    try {
        const int portInt = argc > 1 ? std::stoi(argv[1]) : 12347;
        if (portInt <= 0 || portInt > 65535) {
            std::cerr << "エラー: ポート番号が範囲外です(1-65535): " << argv[1] << "\n";
            return 1;
        }
        const uint16_t port = static_cast<uint16_t>(portInt);

        chat::ChatServer server;
        server.Run(port);
    } catch (const std::exception& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
}
