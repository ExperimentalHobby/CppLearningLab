// 38. 独自バイナリプロトコル - サーバー側
//
// 「コマンドID + データ長 + ペイロード」形式の独自プロトコルで、
// Ping/Pong、Echo、SetValue/GetValue(簡易キーバリューストア)を処理する。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <map>

#include "binary_protocol.h"
#include "tcp_socket.h"
#include "winsock_guard.h"

namespace {

void HandleConnection(net::TcpConnection& connection) {
    std::map<std::string, std::string> store;

    proto::FrameParser parser([&](const proto::Message& message) {
        switch (message.command) {
            case proto::Command::kPing: {
                std::cout << "受信: Ping\n";
                connection.Send(proto::Serialize({proto::Command::kPong, ""}));
                break;
            }
            case proto::Command::kEcho: {
                std::cout << "受信: Echo(" << message.payload << ")\n";
                connection.Send(proto::Serialize({proto::Command::kEcho, message.payload}));
                break;
            }
            case proto::Command::kSetValue: {
                std::string key;
                std::string value;
                proto::DecodeKeyValue(message.payload, key, value);
                std::cout << "受信: SetValue(" << key << " = " << value << ")\n";
                store[key] = value;
                connection.Send(proto::Serialize({proto::Command::kValueResult,
                                                   proto::EncodeValueResult(true, value)}));
                break;
            }
            case proto::Command::kGetValue: {
                const std::string key = proto::DecodeKey(message.payload);
                std::cout << "受信: GetValue(" << key << ")\n";
                const auto it = store.find(key);
                const bool found = it != store.end();
                connection.Send(proto::Serialize(
                    {proto::Command::kValueResult, proto::EncodeValueResult(found, found ? it->second : "")}));
                break;
            }
            default:
                std::cout << "未知のコマンドを無視しました: " << static_cast<int>(message.command) << "\n";
                break;
        }
    });

    for (;;) {
        const std::string chunk = connection.ReceiveSome();
        if (chunk.empty()) {
            std::cout << "クライアントが切断しました。\n";
            return;
        }
        parser.Feed(chunk);
    }
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    const uint16_t port = static_cast<uint16_t>(argc > 1 ? std::stoi(argv[1]) : 12349);

    try {
        net::WinsockGuard guard;
        net::TcpListener listener;
        listener.Listen(port);
        std::cout << "BinaryProtocolServer: ポート " << port << " で待受中...\n";

        for (;;) {
            net::TcpConnection connection = listener.Accept();
            std::cout << "クライアントが接続しました。\n";
            try {
                HandleConnection(connection);
            } catch (const proto::ProtocolError& e) {
                std::cout << "プロトコルエラー: " << e.what() << "\n";
            } catch (const net::TcpError&) {
                std::cout << "通信エラーにより切断されました。\n";
            }
        }
    } catch (const net::TcpError& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
}
