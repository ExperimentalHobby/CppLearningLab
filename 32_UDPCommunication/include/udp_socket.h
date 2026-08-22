// UDPソケットのRAIIラッパー。
//
// TCPと違い`connect`せずに`sendto`/`recvfrom`だけでやり取りできる
// (コネクションレス)点が最大の違い。1回の`sendto`が1つの独立したデータグラム
// になり、TCPのようなバイトストリームの結合/分割は起きない。
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace net {

class UdpError : public std::runtime_error {
   public:
    explicit UdpError(const std::string& message) : std::runtime_error(message) {}
};

// SetReceiveTimeout()で設定した時間内にデータグラムが届かなかった場合に
// ReceiveFromから投げられる。
class UdpTimeoutError : public UdpError {
   public:
    UdpTimeoutError() : UdpError("応答がタイムアウトしました(パケットロストの可能性)") {}
};

using SocketHandle = std::uintptr_t;

// 送信元/宛先を表す最小限の情報(IPv4:port)。
struct Endpoint {
    std::string address;
    uint16_t port = 0;
};

class UdpSocket {
   public:
    UdpSocket();
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    // サーバー側: 指定ポートで受信できるようにbindする。
    void Bind(uint16_t port);

    // ReceiveFromが指定ミリ秒待っても何も届かない場合にUdpTimeoutErrorを
    // 投げるようにする。UDPには到達保証が無いため、クライアント側で
    // 「応答が返ってこない」ケースをハングせず検知できるようにするために使う。
    void SetReceiveTimeout(int milliseconds);

    // dataをdestへ1つのデータグラムとして送信する。
    void SendTo(const std::string& data, const Endpoint& dest);

    // 1つのデータグラムを受信する。送信元(from)も取得する。
    // 戻り値は受信したバイト数。
    size_t ReceiveFrom(std::string& outData, Endpoint& from, size_t maxSize = 4096);

    void Close();

   private:
    SocketHandle handle_;
    static constexpr SocketHandle kInvalidHandle = static_cast<SocketHandle>(-1);
};

}  // namespace net
