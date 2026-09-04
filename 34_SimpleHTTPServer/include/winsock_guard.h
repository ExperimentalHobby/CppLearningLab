// WinsockGuard: WSAStartup/WSACleanupをRAIIで管理する。
//
// Winsock APIを使う前にプロセス内で一度だけWSAStartupを呼ぶ必要があり、
// 使い終わったらWSACleanupを呼ぶ必要がある。mainの先頭でこのクラスの
// インスタンスを1つ作るだけで、例外発生時も含めて後始末を保証する
// (04_PointersAndMemoryのScopedResourceと同じRAIIパターン)。
#pragma once

#include <stdexcept>
#include <string>

namespace net {

class WinsockError : public std::runtime_error {
   public:
    explicit WinsockError(const std::string& message) : std::runtime_error(message) {}
};

class WinsockGuard {
   public:
    WinsockGuard();
    ~WinsockGuard();

    WinsockGuard(const WinsockGuard&) = delete;
    WinsockGuard& operator=(const WinsockGuard&) = delete;
};

}  // namespace net
