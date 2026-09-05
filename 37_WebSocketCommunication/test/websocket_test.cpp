#include "websocket.h"

#include <gtest/gtest.h>

using ws::ComputeAcceptKey;

// SendTextFrame/ReceiveTextFrame/PerformServerHandshake/PerformClientHandshakeは
// net::TcpConnection&に依存する(実ソケット通信が前提)ため自動テスト対象外とし、
// ネットワークに依存しないComputeAcceptKey(SHA-1+Base64)のみテストする。

// RFC 6455 セクション1.3に掲載されているハンドシェイクの例をそのまま使う。
TEST(ComputeAcceptKeyTest, MatchesRfc6455Example) {
    EXPECT_EQ(ComputeAcceptKey("dGhlIHNhbXBsZSBub25jZQ=="), "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
}

// 入力(Sec-WebSocket-Key)が変われば結果も変わる(固定値を返しているわけではないことの確認)。
TEST(ComputeAcceptKeyTest, DifferentKeysProduceDifferentResults) {
    const std::string result1 = ComputeAcceptKey("dGhlIHNhbXBsZSBub25jZQ==");
    const std::string result2 = ComputeAcceptKey("AnotherRandomKey123==");

    EXPECT_NE(result1, result2);
}
