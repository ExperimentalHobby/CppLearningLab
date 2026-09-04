// ChatServer: 複数クライアントを同時に処理する簡易チャットサーバー。
//
// 接続を受け付けるスレッドとは別に、クライアントごとに受信専用スレッドを
// 立てる(スレッドプールではなく単純な「クライアント数分だけスレッドを作る」
// モデル。学習目的のシンプルさを優先している)。接続中クライアント一覧は
// std::mutexで保護し、誰かの発言を他の全クライアントへブロードキャストする。
#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "tcp_socket.h"

namespace chat {

class ChatServer {
   public:
    // 指定ポートで待受を開始し、1接続ごとにスレッドを立てて処理し続ける
    // (呼び出したスレッドをブロックする)。
    void Run(uint16_t port);

   private:
    struct Client {
        net::TcpConnection connection;
        std::string nickname;
    };

    std::mutex mutex_;
    std::vector<std::shared_ptr<Client>> clients_;

    void HandleClient(std::shared_ptr<Client> client);

    // exceptを除く全クライアントへmessageを送る。
    // mutex_を保持したまま送信するため、ブロードキャスト中は他の
    // 接続/切断/送信処理と直列化される(シンプルさ優先の設計)。
    void Broadcast(const std::string& message, const Client* except);

    void RemoveClient(const Client* target);
};

}  // namespace chat
