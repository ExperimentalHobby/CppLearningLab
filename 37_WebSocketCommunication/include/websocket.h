// RFC 6455 WebSocketのハンドシェイクと最小限のフレーミングを、
// 31番のTCP実装の上に自作する(Boost.Beast/websocketppが本開発環境に
// 無いため)。SHA-1計算のみWindows標準のCNG(BCrypt) APIを使う。
//
// スコープの都合で以下は非対応(README参照):
//   - メッセージのフラグメンテーション(FIN=0の継続フレーム)
//   - pingpong/close以外の制御フレームの明示的な処理
//   - 拡張(permessage-deflate等)
#pragma once

#include <stdexcept>
#include <string>

#include "tcp_socket.h"

namespace ws {

class WebSocketError : public std::runtime_error {
   public:
    explicit WebSocketError(const std::string& message) : std::runtime_error(message) {}
};

// "Sec-WebSocket-Key"ヘッダーの値から、RFC 6455で定められた固定GUIDと
// 連結してSHA-1し、Base64エンコードした"Sec-WebSocket-Accept"値を計算する。
// SHA-1計算にはWindows標準のBCrypt APIを使う。
std::string ComputeAcceptKey(const std::string& secWebSocketKey);

// サーバー側: HTTPアップグレードリクエストを読み取り、HTTP 101レスポンスを
// 返す。成功すればconnectionはWebSocketとして送受信できる状態になる。
void PerformServerHandshake(net::TcpConnection& connection);

// クライアント側: hostのpathへWebSocketハンドシェイクを行う。
// レスポンスのSec-WebSocket-Acceptが期待値と一致しない場合は例外を投げる。
void PerformClientHandshake(net::TcpConnection& connection, const std::string& host, const std::string& path);

// テキストフレームを1つ送信する。RFC 6455により、クライアント→サーバーの
// フレームは必ずマスクする必要があり、サーバー→クライアントは
// マスクしてはならない。maskedにはその向きを渡す。
void SendTextFrame(net::TcpConnection& connection, const std::string& text, bool masked);

// テキストフレームを1つ受信する(フラグメンテーション無しの単一フレームのみ対応)。
std::string ReceiveTextFrame(net::TcpConnection& connection);

}  // namespace ws
