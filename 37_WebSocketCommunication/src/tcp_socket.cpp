#include "tcp_socket.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <utility>

namespace net {

namespace {

std::string LastErrorMessage() {
    return "WSAエラーコード=" + std::to_string(WSAGetLastError());
}

}  // namespace

// --- TcpConnection ---

TcpConnection::TcpConnection() = default;

TcpConnection::TcpConnection(SocketHandle handle) : handle_(handle) {}

TcpConnection::~TcpConnection() { Close(); }

TcpConnection::TcpConnection(TcpConnection&& other) noexcept
    : handle_(other.handle_), recvBuffer_(std::move(other.recvBuffer_)) {
    other.handle_ = kInvalidHandle;
}

TcpConnection& TcpConnection::operator=(TcpConnection&& other) noexcept {
    if (this != &other) {
        Close();
        handle_ = other.handle_;
        recvBuffer_ = std::move(other.recvBuffer_);
        other.handle_ = kInvalidHandle;
    }
    return *this;
}

TcpConnection TcpConnection::Connect(const std::string& host, uint16_t port) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* resolved = nullptr;
    const std::string portStr = std::to_string(port);
    if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &resolved) != 0) {
        throw TcpError("ホスト名の解決に失敗しました: " + host);
    }

    SOCKET sock = INVALID_SOCKET;
    for (addrinfo* p = resolved; p != nullptr; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == INVALID_SOCKET) {
            continue;
        }
        if (connect(sock, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0) {
            break;  // 接続成功
        }
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    freeaddrinfo(resolved);

    if (sock == INVALID_SOCKET) {
        throw TcpError("接続に失敗しました(" + host + ":" + portStr + "): " + LastErrorMessage());
    }
    return TcpConnection(static_cast<SocketHandle>(sock));
}

void TcpConnection::Send(const std::string& data) {
    const SOCKET sock = static_cast<SOCKET>(handle_);
    size_t sentTotal = 0;
    while (sentTotal < data.size()) {
        const int sent =
            send(sock, data.data() + sentTotal, static_cast<int>(data.size() - sentTotal), 0);
        if (sent == SOCKET_ERROR) {
            throw TcpError("送信に失敗しました: " + LastErrorMessage());
        }
        sentTotal += static_cast<size_t>(sent);
    }
}

void TcpConnection::SendLine(const std::string& line) { Send(line + "\n"); }

bool TcpConnection::FillBuffer() {
    char buffer[4096];
    const SOCKET sock = static_cast<SOCKET>(handle_);
    const int received = recv(sock, buffer, static_cast<int>(sizeof(buffer)), 0);
    if (received == SOCKET_ERROR) {
        throw TcpError("受信に失敗しました: " + LastErrorMessage());
    }
    if (received == 0) {
        return false;  // 相手が接続をクローズした
    }
    recvBuffer_.append(buffer, static_cast<size_t>(received));
    return true;
}

bool TcpConnection::ReceiveLine(std::string& outLine) {
    for (;;) {
        const size_t newlinePos = recvBuffer_.find('\n');
        if (newlinePos != std::string::npos) {
            outLine = recvBuffer_.substr(0, newlinePos);
            if (!outLine.empty() && outLine.back() == '\r') {
                outLine.pop_back();  // "\r\n"改行にも対応
            }
            recvBuffer_.erase(0, newlinePos + 1);
            return true;
        }
        if (!FillBuffer()) {
            // 接続が閉じられた。バッファに未完成の行が残っていれば最後の1行として返す。
            if (!recvBuffer_.empty()) {
                outLine = recvBuffer_;
                recvBuffer_.clear();
                return true;
            }
            return false;
        }
    }
}

std::string TcpConnection::ReceiveExact(size_t count) {
    while (recvBuffer_.size() < count) {
        if (!FillBuffer()) {
            throw TcpError("接続が閉じられ、必要なバイト数を受信できませんでした(要求=" +
                            std::to_string(count) + ", 受信済み=" + std::to_string(recvBuffer_.size()) + ")");
        }
    }
    const std::string result = recvBuffer_.substr(0, count);
    recvBuffer_.erase(0, count);
    return result;
}

void TcpConnection::Close() {
    if (handle_ != kInvalidHandle) {
        closesocket(static_cast<SOCKET>(handle_));
        handle_ = kInvalidHandle;
    }
}

// --- TcpListener ---

TcpListener::TcpListener() = default;

TcpListener::~TcpListener() { Close(); }

void TcpListener::Listen(uint16_t port, int backlog) {
    const SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        throw TcpError("ソケットの作成に失敗しました: " + LastErrorMessage());
    }

    const BOOL reuseAddr = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuseAddr), sizeof(reuseAddr));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        const std::string message = "bindに失敗しました(port=" + std::to_string(port) + "): " + LastErrorMessage();
        closesocket(sock);
        throw TcpError(message);
    }
    if (listen(sock, backlog) == SOCKET_ERROR) {
        const std::string message = "listenに失敗しました: " + LastErrorMessage();
        closesocket(sock);
        throw TcpError(message);
    }

    handle_ = static_cast<SocketHandle>(sock);
}

TcpConnection TcpListener::Accept() {
    const SOCKET listenSock = static_cast<SOCKET>(handle_);
    const SOCKET clientSock = accept(listenSock, nullptr, nullptr);
    if (clientSock == INVALID_SOCKET) {
        throw TcpError("acceptに失敗しました: " + LastErrorMessage());
    }
    return TcpConnection(static_cast<SocketHandle>(clientSock));
}

void TcpListener::Close() {
    if (handle_ != TcpConnection::kInvalidHandle) {
        closesocket(static_cast<SOCKET>(handle_));
        handle_ = TcpConnection::kInvalidHandle;
    }
}

}  // namespace net
