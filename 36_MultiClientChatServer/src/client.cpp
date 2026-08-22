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
    try {
        std::string line;
        while (g_running && connection->ReceiveLine(line)) {
            std::cout << line << "\n";
        }
    } catch (const std::exception&) {
        // 切断やShutdownによる受信エラーは正常な停止として扱う。
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
    const std::string nickname = argv[3];

    try {
        const int portInt = std::stoi(argv[2]);
        if (portInt <= 0 || portInt > 65535) {
            std::cerr << "エラー: ポート番号が範囲外です(1-65535): " << argv[2] << "\n";
            return 1;
        }
        const uint16_t port = static_cast<uint16_t>(portInt);

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
        connection.Shutdown();  // ReceiveLineのブロックを解除するため能動的にshutdown
        receiver.join();
        connection.Close();
    } catch (const std::exception& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
