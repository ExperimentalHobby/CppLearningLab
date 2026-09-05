#include "http_message.h"

#include <gtest/gtest.h>

using httpsrv::BuildHttpResponse;

// ReadHttpRequestはnet::TcpConnection&に依存する(実ソケット通信が前提)ため
// 自動テスト対象外とし、TcpConnectionに依存しないBuildHttpResponseのみテストする。

TEST(BuildHttpResponseTest, IncludesStatusLine) {
    const std::string response = BuildHttpResponse(200, "OK", "text/plain", "hello");

    EXPECT_EQ(response.substr(0, response.find("\r\n")), "HTTP/1.1 200 OK");
}

TEST(BuildHttpResponseTest, IncludesContentTypeHeader) {
    const std::string response = BuildHttpResponse(200, "OK", "text/plain; charset=utf-8", "hello");

    EXPECT_NE(response.find("Content-Type: text/plain; charset=utf-8\r\n"), std::string::npos);
}

// Content-Lengthはバイト数(文字数ではない)で計算される。
TEST(BuildHttpResponseTest, ContentLengthMatchesBodyByteSize) {
    const std::string body = "こんにちは";  // UTF-8で15バイト
    const std::string response = BuildHttpResponse(200, "OK", "text/plain; charset=utf-8", body);

    EXPECT_NE(response.find("Content-Length: 15\r\n"), std::string::npos);
}

// ヘッダーとボディの間は空行("\r\n\r\n")で区切られる。
TEST(BuildHttpResponseTest, SeparatesHeadersFromBodyWithBlankLine) {
    const std::string response = BuildHttpResponse(200, "OK", "text/plain", "hello");

    EXPECT_NE(response.find("\r\n\r\nhello"), std::string::npos);
}

TEST(BuildHttpResponseTest, IncludesConnectionCloseHeader) {
    const std::string response = BuildHttpResponse(200, "OK", "text/plain", "hello");

    EXPECT_NE(response.find("Connection: close\r\n"), std::string::npos);
}

TEST(BuildHttpResponseTest, ReflectsNonOkStatusCode) {
    const std::string response = BuildHttpResponse(404, "Not Found", "text/plain", "not found");

    EXPECT_EQ(response.substr(0, response.find("\r\n")), "HTTP/1.1 404 Not Found");
}
