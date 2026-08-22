#include "udp_socket.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <vector>

// mstcpip.hのSIO_UDP_CONNRESETは_WIN32_WINNTのバージョンガードの都合で
// 期待通りに見えないことがあるため、既知の値を直接定義する
// (定義: _WSAIOW(IOC_VENDOR, 12) = IOC_IN | IOC_VENDOR | 12)。
#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif

namespace net {

namespace {

std::string LastErrorMessage() { return "WSAエラーコード=" + std::to_string(WSAGetLastError()); }

}  // namespace

UdpSocket::UdpSocket() {
    const SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        throw UdpError("ソケットの作成に失敗しました: " + LastErrorMessage());
    }

    // Windows固有の挙動への対処: UDPは本来コネクションレスだが、Windowsでは
    // 「以前sendtoした宛先からICMP Port Unreachableが返ってきた」場合に、
    // 次のrecvfromがWSAECONNRESET(10054)で失敗するというTCPライクな挙動をする
    // (他のOSには無いWindows特有の仕様)。これを無効化しないと、応答が無い
    // 相手に送った際に「タイムアウト」ではなく「エラー」になってしまい、
    // UDPの「相手の状態を気にせず送りっぱなしにできる」という性質と食い違う
    // 見え方になる。SIO_UDP_CONNRESETで無効化することで、応答が無い場合は
    // 素直にSetReceiveTimeout()で設定したタイムアウトが働くようにする。
    BOOL enableConnReset = FALSE;
    DWORD bytesReturned = 0;
    if (WSAIoctl(sock, SIO_UDP_CONNRESET, &enableConnReset, sizeof(enableConnReset), nullptr, 0, &bytesReturned,
                 nullptr, nullptr) == SOCKET_ERROR) {
        const int err = WSAGetLastError();
        closesocket(sock);
        throw UdpError("SIO_UDP_CONNRESETの無効化に失敗しました: WSAエラーコード=" + std::to_string(err));
    }

    handle_ = static_cast<SocketHandle>(sock);
}

UdpSocket::~UdpSocket() { Close(); }

void UdpSocket::Bind(uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(static_cast<SOCKET>(handle_), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) ==
        SOCKET_ERROR) {
        throw UdpError("bindに失敗しました(port=" + std::to_string(port) + "): " + LastErrorMessage());
    }
}

void UdpSocket::SetReceiveTimeout(int milliseconds) {
    const DWORD timeout = static_cast<DWORD>(milliseconds);
    if (setsockopt(static_cast<SOCKET>(handle_), SOL_SOCKET, SO_RCVTIMEO,
                    reinterpret_cast<const char*>(&timeout), sizeof(timeout)) == SOCKET_ERROR) {
        throw UdpError("受信タイムアウトの設定に失敗しました: " + LastErrorMessage());
    }
}

void UdpSocket::SendTo(const std::string& data, const Endpoint& dest) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(dest.port);
    if (inet_pton(AF_INET, dest.address.c_str(), &addr.sin_addr) != 1) {
        throw UdpError("宛先アドレスの解釈に失敗しました: " + dest.address);
    }

    // UDPは「1回のsendto = 1個のデータグラム」であり、TCPのようにバイト列が
    // 結合されたり分割されたりしない(コネクションレス通信の特徴)。
    const int sent = sendto(static_cast<SOCKET>(handle_), data.data(), static_cast<int>(data.size()), 0,
                             reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (sent == SOCKET_ERROR) {
        throw UdpError("送信に失敗しました: " + LastErrorMessage());
    }
}

size_t UdpSocket::ReceiveFrom(std::string& outData, Endpoint& from, size_t maxSize) {
    std::vector<char> buffer(maxSize);
    sockaddr_in senderAddr{};
    int senderAddrLen = sizeof(senderAddr);

    const int received =
        recvfrom(static_cast<SOCKET>(handle_), buffer.data(), static_cast<int>(buffer.size()), 0,
                 reinterpret_cast<sockaddr*>(&senderAddr), &senderAddrLen);
    if (received == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAETIMEDOUT) {
            throw UdpTimeoutError();
        }
        throw UdpError("受信に失敗しました: " + LastErrorMessage());
    }

    outData.assign(buffer.data(), static_cast<size_t>(received));

    char addrText[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &senderAddr.sin_addr, addrText, sizeof(addrText));
    from.address = addrText;
    from.port = ntohs(senderAddr.sin_port);

    return static_cast<size_t>(received);
}

void UdpSocket::Close() {
    if (handle_ != kInvalidHandle) {
        closesocket(static_cast<SOCKET>(handle_));
        handle_ = kInvalidHandle;
    }
}

}  // namespace net
