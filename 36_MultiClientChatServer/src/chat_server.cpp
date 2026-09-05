#include "chat_server.h"

#include <algorithm>
#include <iostream>
#include <thread>

#include "winsock_guard.h"

namespace chat {

void ChatServer::Broadcast(const std::string& message, const Client* except) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const std::shared_ptr<Client>& client : clients_) {
        if (client.get() == except) {
            continue;
        }
        try {
            client->connection.SendLine(message);
        } catch (const net::TcpError&) {
            // 送信先が既に切断している等は無視する(その接続の受信スレッド側で
            // 検知され、後始末される)。
        }
    }
}

void ChatServer::RemoveClient(const Client* target) {
    std::lock_guard<std::mutex> lock(mutex_);
    clients_.erase(std::remove_if(clients_.begin(), clients_.end(),
                                   [target](const std::shared_ptr<Client>& c) { return c.get() == target; }),
                   clients_.end());
}

void ChatServer::HandleClient(std::shared_ptr<Client> client) {
    // 接続直後の最初の1行をニックネームとして扱う。
    // ニックネーム受信失敗やその後の例外時も必ずRemoveClientする。
    try {
        if (!client->connection.ReceiveLine(client->nickname) || client->nickname.empty()) {
            RemoveClient(client.get());
            return;
        }
        std::cout << "[" << client->nickname << "] が参加しました。\n";
        Broadcast(client->nickname + " が参加しました。", client.get());

        std::string line;
        while (client->connection.ReceiveLine(line)) {
            std::cout << "[" << client->nickname << "] " << line << "\n";
            Broadcast(client->nickname + ": " + line, client.get());
        }
    } catch (const std::exception& e) {
        std::cerr << "[" << client->nickname << "] 例外: " << e.what() << "\n";
    }

    std::cout << "[" << client->nickname << "] が退出しました。\n";
    RemoveClient(client.get());
    Broadcast(client->nickname + " が退出しました。", client.get());
}

void ChatServer::Run(uint16_t port) {
    net::WinsockGuard guard;
    net::TcpListener listener;
    listener.Listen(port);
    std::cout << "MultiClientChatServer: ポート " << port << " で待受中...\n";

    for (;;) {
        net::TcpConnection connection = listener.Accept();
        auto client = std::make_shared<Client>();
        client->connection = std::move(connection);

        {
            // 受信専用スレッドを立てる前にclients_へ登録しておく必要がある。
            // 逆順にすると、登録前に他クライアントからのBroadcastが届いた場合に
            // この接続が一覧から漏れてしまう(競合状態を避けるための順序)。
            std::lock_guard<std::mutex> lock(mutex_);
            clients_.push_back(client);
        }

        std::thread(&ChatServer::HandleClient, this, client).detach();
    }
}

}  // namespace chat
