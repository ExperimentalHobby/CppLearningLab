// Base64エンコード。WebSocketハンドシェイクのSec-WebSocket-Key/Accept
// ヘッダー値の組み立てに使う。暗号ではなく単純なバイト列変換のため自作する。
#pragma once

#include <string>

namespace ws {

std::string Base64Encode(const std::string& data);

}  // namespace ws
