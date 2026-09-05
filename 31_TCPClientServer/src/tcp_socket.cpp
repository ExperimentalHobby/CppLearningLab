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

TcpConnection::TcpConnection(size_t maxLineLength) : maxLineLength_(maxLineLength) {}

TcpConnection::TcpConnection(SocketHandle handle, size_t maxLineLength)
    : handle_(handle), maxLineLength_(maxLineLength) {}

TcpConnection::~TcpConnection() { Close(); }

TcpConnection::TcpConnection(TcpConnection&& other) noexcept
    : handle_(other.handle_), recvBuffer_(std::move(other.recvBuffer_)),
      maxLineLength_(other.maxLineLength_) {
    other.handle_ = kInvalidHandle;
}

TcpConnection& TcpConnection::operator=(TcpConnection&& other) noexcept {
    if (this != &other) {
        Close();
        handle_ = other.handle_;
        recvBuffer_ = std::move(other.recvBuffer_);
        maxLineLength_ = other.maxLineLength_;
        other.handle_ = kInvalidHandle;
    }
    return *this;
}

TcpConnection TcpConnection::Connect(const std::string& host, uint16_t port,
                                      size_t maxLineLength) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* resolved = nullptr;
    const std::string portStr = std::to_string(port);
    const int gaiErr = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &resolved);
    if (gaiErr != 0) {
        throw TcpConnectError("ホスト名の解決に失敗しました(" + host + ":" + portStr + "): " +
                              std::string(gai_strerrorA(gaiErr)));
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
        throw TcpConnectError("接続に失敗しました(" + host + ":" + portStr + "): " + LastErrorMessage());
    }
    return TcpConnection(static_cast<SocketHandle>(sock), maxLineLength);
}

void TcpConnection::Send(const std::string& data) {
    const SOCKET sock = static_cast<SOCKET>(handle_);
    size_t sentTotal = 0;
    // 1回のsend()がdata全体を送り切る保証は無い(送信バッファの空き次第で
    // 部分的にしか送れないことがある)ため、送りきるまでループする。
    while (sentTotal < data.size()) {
        const size_t remaining = data.size() - sentTotal;
        constexpr int kMaxSendLen = 0x7fffffff;  // send()のlenはint
        const int toSend = remaining > static_cast<size_t>(kMaxSendLen)
                               ? kMaxSendLen
                               : static_cast<int>(remaining);
        const int sent = send(sock, data.data() + sentTotal, toSend, 0);
        if (sent == SOCKET_ERROR) {
            throw TcpSendError("送信に失敗しました: " + LastErrorMessage());
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
        throw TcpReceiveError("受信に失敗しました: " + LastErrorMessage());
    }
    if (received == 0) {
        return false;  // 相手が接続をクローズした
    }
    const size_t newTotal = recvBuffer_.size() + static_cast<size_t>(received);
    if (newTotal > maxLineLength_) {
        throw TcpLineTooLongError(newTotal, maxLineLength_);
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
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuseAddr),
                   sizeof(reuseAddr)) == SOCKET_ERROR) {
        const std::string message = "setsockopt(SO_REUSEADDR)に失敗しました: " + LastErrorMessage();
        closesocket(sock);
        throw TcpError(message);
    }
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

TcpConnection TcpListener::Accept(size_t maxLineLength) {
    const SOCKET listenSock = static_cast<SOCKET>(handle_);
    const SOCKET clientSock = accept(listenSock, nullptr, nullptr);
    if (clientSock == INVALID_SOCKET) {
        throw TcpConnectError("acceptに失敗しました: " + LastErrorMessage());
    }
    return TcpConnection(static_cast<SocketHandle>(clientSock), maxLineLength);
}

void TcpListener::Close() {
    if (handle_ != TcpConnection::kInvalidHandle) {
        closesocket(static_cast<SOCKET>(handle_));
        handle_ = TcpConnection::kInvalidHandle;
    }
}

}  // namespace net
