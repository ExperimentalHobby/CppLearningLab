// TCPソケットのRAIIラッパー。
//
// 「TCPはバイトストリームであり、メッセージの境界(どこまでが1つの
// メッセージか)はアプリケーション側で決める必要がある」という学習ポイントを
// 実装で示すため、TcpConnectionは生のSend/Receiveに加えて、改行区切りで
// 1メッセージとする最小限のフレーミング(SendLine/ReceiveLine)を提供する。
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace net {

class TcpError : public std::runtime_error {
   public:
    explicit TcpError(const std::string& message) : std::runtime_error(message) {}
};

// socket()の実体をuintptr_tで保持し、windows.h/winsock2.hをこのヘッダーの
// 利用側に強制しないようにしている(SOCKET型はwinsock2.hが無いと定義されない)。
using SocketHandle = std::uintptr_t;

class TcpConnection {
   public:
    TcpConnection();
    explicit TcpConnection(SocketHandle handle);
    ~TcpConnection();

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;
    TcpConnection(TcpConnection&& other) noexcept;
    TcpConnection& operator=(TcpConnection&& other) noexcept;

    // host:portへTCP接続する。
    static TcpConnection Connect(const std::string& host, uint16_t port);

    // 生のバイト列を送信する。
    void Send(const std::string& data);

    // dataの末尾に'\n'を付けて送信する。
    void SendLine(const std::string& line);

    // 内部バッファと受信済みデータから改行区切りの1行を取り出す。
    // 行が取り出せた場合はtrueを返しoutLineに格納する。
    // 相手が接続を閉じ、未完成の行も無い場合はfalseを返す。
    bool ReceiveLine(std::string& outLine);

    // ちょうどcount バイトを読み取って返す(WebSocketフレームのように、
    // 改行に依存しないバイナリのメッセージ境界を扱うために使う)。
    // 相手が接続を閉じてcountバイトに満たない場合はTcpErrorを投げる。
    std::string ReceiveExact(size_t count);

    void Close();
    bool IsValid() const { return handle_ != kInvalidHandle; }

    static constexpr SocketHandle kInvalidHandle = static_cast<SocketHandle>(-1);

   private:
    SocketHandle handle_ = kInvalidHandle;
    std::string recvBuffer_;

    bool FillBuffer();  // recv()で追加データを読み込む。切断されたらfalse。
};

class TcpListener {
   public:
    TcpListener();
    ~TcpListener();

    TcpListener(const TcpListener&) = delete;
    TcpListener& operator=(const TcpListener&) = delete;

    // 全インターフェース(0.0.0.0)のportで待受を開始する(bind+listen)。
    void Listen(uint16_t port, int backlog = 16);

    // 1接続をaccept()で受け入れる。
    TcpConnection Accept();

    void Close();

   private:
    SocketHandle handle_ = TcpConnection::kInvalidHandle;
};

}  // namespace net
