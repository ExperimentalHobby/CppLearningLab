// WinHTTPを使ったHTTPS GETクライアント。
//
// 推奨ライブラリのlibcurlが本開発環境に無いため、Windows標準の高レベル
// HTTPクライアントAPIである**WinHTTP**(`winhttp.lib`)を使う
// (詳細はルートREADMEの「通信(31-38番台)で採用したライブラリについて」を参照)。
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace http {

class HttpError : public std::runtime_error {
   public:
    explicit HttpError(const std::string& message) : std::runtime_error(message) {}
};

struct HttpResponse {
    int statusCode = 0;
    std::string body;
};

// host(例: "api.github.com")のpath(例: "/repos/octocat/Hello-World")へ
// HTTPS GETリクエストを送る。userAgentはGitHub API等、多くのAPIで必須の
// ヘッダーのため明示的に指定させる。
HttpResponse HttpsGet(const std::string& host, const std::string& path, const std::string& userAgent);

}  // namespace http
