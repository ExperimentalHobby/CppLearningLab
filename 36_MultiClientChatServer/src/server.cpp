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
    const uint16_t port = static_cast<uint16_t>(argc > 1 ? std::stoi(argv[1]) : 12347);

    try {
        chat::ChatServer server;
        server.Run(port);
    } catch (const net::TcpError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
}
