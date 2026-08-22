#include "winsock_guard.h"

#include <winsock2.h>

namespace net {

WinsockGuard::WinsockGuard() {
    WSADATA wsaData{};
    const int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        throw WinsockError("WSAStartupに失敗しました: エラーコード=" + std::to_string(result));
    }
}

WinsockGuard::~WinsockGuard() { WSACleanup(); }

}  // namespace net
