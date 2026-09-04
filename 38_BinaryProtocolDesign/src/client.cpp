// 38. 独自バイナリプロトコル - クライアント側
//
// Ping/Pong、Echo、SetValue/GetValueの一連のやり取りを行う。
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "binary_protocol.h"
#include "tcp_socket.h"
#include "winsock_guard.h"

namespace {

// 1件分の応答メッセージが揃うまで受信し続ける。TCPの部分受信特性上、
// 1回のReceiveSome()で必ず1メッセージ分そろうとは限らないため、
// FrameParserにフィードし続けてキューにメッセージが積まれるのを待つ。
proto::Message ReceiveOneMessage(net::TcpConnection& connection, proto::FrameParser& parser,
                                  std::vector<proto::Message>& queue) {
    while (queue.empty()) {
        const std::string chunk = connection.ReceiveSome();
        if (chunk.empty()) {
            throw net::TcpError("応答を受信する前に接続が閉じられました");
        }
        parser.Feed(chunk);
    }
    proto::Message message = queue.front();
    queue.erase(queue.begin());
    return message;
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    if (argc < 3) {
        std::cerr << "使い方: " << argv[0] << " <host> <port>\n";
        return 1;
    }
    const std::string host = argv[1];

    try {
        int parsedPort = 0;
        try {
            parsedPort = std::stoi(argv[2]);
            // windows.hがmax/minをマクロとして定義していることがあるため、
            // std::numeric_limitsのmax()がマクロ展開されないよう括弧で保護する。
            if (parsedPort < 0 || parsedPort > (std::numeric_limits<uint16_t>::max)()) {
                throw std::out_of_range("port out of range");
            }
        } catch (const std::exception&) {
            throw std::runtime_error("ポート番号は0から65535の整数で指定してください。");
        }
        const uint16_t port = static_cast<uint16_t>(parsedPort);

        net::WinsockGuard guard;
        net::TcpConnection connection = net::TcpConnection::Connect(host, port);

        std::vector<proto::Message> queue;
        proto::FrameParser parser([&](const proto::Message& message) { queue.push_back(message); });

        // --- Ping/Pong ---
        connection.Send(proto::Serialize({proto::Command::kPing, ""}));
        const proto::Message pong = ReceiveOneMessage(connection, parser, queue);
        std::cout << "Ping -> " << (pong.command == proto::Command::kPong ? "Pong received" : "unexpected")
                  << "\n";

        // --- Echo ---
        connection.Send(proto::Serialize({proto::Command::kEcho, "Hello, Binary Protocol!"}));
        const proto::Message echoed = ReceiveOneMessage(connection, parser, queue);
        std::cout << "Echo -> " << echoed.payload << "\n";

        // --- SetValue ---
        connection.Send(
            proto::Serialize({proto::Command::kSetValue, proto::EncodeKeyValue("name", "Alice")}));
        const proto::Message setResult = ReceiveOneMessage(connection, parser, queue);
        bool found = false;
        std::string value;
        proto::DecodeValueResult(setResult.payload, found, value);
        std::cout << "SetValue(name, Alice) -> found=" << found << " value=" << value << "\n";

        // --- GetValue(存在するキー) ---
        connection.Send(proto::Serialize({proto::Command::kGetValue, proto::EncodeKey("name")}));
        const proto::Message getResult = ReceiveOneMessage(connection, parser, queue);
        proto::DecodeValueResult(getResult.payload, found, value);
        std::cout << "GetValue(name) -> found=" << found << " value=" << value << "\n";

        // --- GetValue(存在しないキー) ---
        connection.Send(proto::Serialize({proto::Command::kGetValue, proto::EncodeKey("missing")}));
        const proto::Message missingResult = ReceiveOneMessage(connection, parser, queue);
        proto::DecodeValueResult(missingResult.payload, found, value);
        std::cout << "GetValue(missing) -> found=" << found << "\n";
    } catch (const net::TcpError& e) {
        std::cerr << "TCPエラー: " << e.what() << "\n";
        return 1;
    } catch (const proto::ProtocolError& e) {
        std::cerr << "プロトコルエラー: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "エラー: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
