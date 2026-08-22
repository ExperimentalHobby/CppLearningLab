// 36. マルチクライアントチャットサーバー - クライアント側
//
// 接続してニックネームを送った後、受信専用スレッドでブロードキャストされた
// メッセージを表示しつつ、メインスレッドは標準入力から読んだ行を送信し続ける。
#ifdef _WIN32
#include <windows.h>
#endif

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

#include "tcp_socket.h"
#include "winsock_guard.h"

namespace {

std::atomic<bool> g_running{true};

void ReceiveLoop(net::TcpConnection* connection) {
    std::string line;
    while (g_running && connection->ReceiveLine(line)) {
        std::cout << line << "\n";
    }
    g_running = false;
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    if (argc < 4) {
        std::cerr << "使い方: " << argv[0] << " <host> <port> <ニックネーム>\n";
        return 1;
    }
    const std::string host = argv[1];
    const uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));
    const std::string nickname = argv[3];

    try {
        net::WinsockGuard guard;
        net::TcpConnection connection = net::TcpConnection::Connect(host, port);
        connection.SendLine(nickname);
        std::cout << nickname << " として接続しました。('quit'で終了)\n";

        std::thread receiver(ReceiveLoop, &connection);

        std::string input;
        while (g_running && std::getline(std::cin, input)) {
            if (input == "quit") {
                break;
            }
            connection.SendLine(input);
        }

        g_running = false;
        connection.Close();  // ReceiveLineのブロックを解除するため能動的にクローズ
        receiver.join();
    } catch (const net::TcpError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
