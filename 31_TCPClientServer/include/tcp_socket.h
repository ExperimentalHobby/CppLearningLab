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

// 基底例外クラス。すべての net:: 例外はここから派生する。
class TcpError : public std::runtime_error {
   public:
    explicit TcpError(const std::string& message) : std::runtime_error(message) {}
};

// 名前解決または接続確立の失敗。
class TcpConnectError : public TcpError {
   public:
    explicit TcpConnectError(const std::string& message) : TcpError(message) {}
};

// send() の失敗。
class TcpSendError : public TcpError {
   public:
    explicit TcpSendError(const std::string& message) : TcpError(message) {}
};

// recv() の失敗。
class TcpReceiveError : public TcpError {
   public:
    explicit TcpReceiveError(const std::string& message) : TcpError(message) {}
};

// 受信行が最大行長を超えた。
// received: 受信済みバイト数、limit: 設定された上限バイト数。
class TcpLineTooLongError : public TcpError {
   public:
    TcpLineTooLongError(size_t received, size_t limit)
        : TcpError("受信行が最大行長を超えました(受信済み=" + std::to_string(received) +
                   " bytes, 上限=" + std::to_string(limit) + " bytes, 超過=" +
                   std::to_string(received - limit) + " bytes)"),
          received_(received),
          limit_(limit) {}

    size_t received() const { return received_; }
    size_t limit() const { return limit_; }

   private:
    size_t received_;
    size_t limit_;
};

// socket()の実体をuintptr_tで保持し、windows.h/winsock2.hをこのヘッダーの
// 利用側に強制しないようにしている(SOCKET型はwinsock2.hが無いと定義されない)。
using SocketHandle = std::uintptr_t;

class TcpConnection {
   public:
    static constexpr size_t kDefaultMaxLineLength = 64 * 1024;

    explicit TcpConnection(size_t maxLineLength = kDefaultMaxLineLength);
    explicit TcpConnection(SocketHandle handle, size_t maxLineLength = kDefaultMaxLineLength);
    ~TcpConnection();

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;
    TcpConnection(TcpConnection&& other) noexcept;
    TcpConnection& operator=(TcpConnection&& other) noexcept;

    // host:portへTCP接続する。maxLineLengthはReceiveLineの最大行長(バイト)。
    static TcpConnection Connect(const std::string& host, uint16_t port,
                                 size_t maxLineLength = kDefaultMaxLineLength);

    // 生のバイト列を送信する。
    void Send(const std::string& data);

    // dataの末尾に'\n'を付けて送信する。
    void SendLine(const std::string& line);

    // 内部バッファと受信済みデータから改行区切りの1行を取り出す。
    // 行が取り出せた場合はtrueを返しoutLineに格納する。
    // 相手が接続を閉じ、未完成の行も無い場合はfalseを返す。
    bool ReceiveLine(std::string& outLine);

    void Close();
    bool IsValid() const { return handle_ != kInvalidHandle; }

    static constexpr SocketHandle kInvalidHandle = static_cast<SocketHandle>(-1);

   private:
    SocketHandle handle_ = kInvalidHandle;
    std::string recvBuffer_;
    size_t maxLineLength_ = kDefaultMaxLineLength;

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

    // 1接続をaccept()で受け入れる。maxLineLengthは受け入れた接続のReceiveLine最大行長。
    TcpConnection Accept(size_t maxLineLength = TcpConnection::kDefaultMaxLineLength);

    void Close();

   private:
    SocketHandle handle_ = TcpConnection::kInvalidHandle;
};

}  // namespace net
