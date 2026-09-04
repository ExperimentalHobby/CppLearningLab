#include "http_message.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace httpsrv {

namespace {

std::string ToLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
    return text;
}

std::string Trim(const std::string& text) {
    const size_t begin = text.find_first_not_of(" \t");
    if (begin == std::string::npos) {
        return "";
    }
    const size_t end = text.find_last_not_of(" \t");
    return text.substr(begin, end - begin + 1);
}

}  // namespace

bool ReadHttpRequest(net::TcpConnection& connection, HttpRequest& outRequest) {
    std::string requestLine;
    if (!connection.ReceiveLine(requestLine)) {
        return false;
    }

    std::istringstream lineStream(requestLine);
    if (!(lineStream >> outRequest.method >> outRequest.path >> outRequest.version)) {
        return false;  // リクエストラインの形式が不正
    }

    outRequest.headers.clear();
    std::string headerLine;
    while (connection.ReceiveLine(headerLine) && !headerLine.empty()) {
        const size_t colonPos = headerLine.find(':');
        if (colonPos == std::string::npos) {
            continue;  // 不正な行はスキップ(HTTPパーサーとしては最小限の割り切り)
        }
        const std::string key = ToLower(Trim(headerLine.substr(0, colonPos)));
        const std::string value = Trim(headerLine.substr(colonPos + 1));
        outRequest.headers[key] = value;
    }

    return true;
}

std::string BuildHttpResponse(int statusCode, const std::string& statusText, const std::string& contentType,
                               const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    return response.str();
}

}  // namespace httpsrv
