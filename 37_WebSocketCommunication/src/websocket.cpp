#include "websocket.h"

#include <windows.h>

#include <bcrypt.h>

#include <algorithm>
#include <cctype>
#include <map>
#include <random>
#include <sstream>
#include <vector>

#include "base64.h"

#pragma comment(lib, "bcrypt.lib")

namespace ws {

namespace {

// RFC 6455で定められた固定のGUID。Sec-WebSocket-Keyと連結してSHA-1する。
constexpr const char* kWebSocketGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

std::string Sha1(const std::string& input) {
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA1_ALGORITHM, nullptr, 0) != 0) {
        throw WebSocketError("BCryptOpenAlgorithmProviderに失敗しました");
    }

    DWORD hashObjectSize = 0;
    DWORD hashLength = 0;
    DWORD resultSize = 0;
    BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&hashObjectSize),
                       sizeof(hashObjectSize), &resultSize, 0);
    BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&hashLength), sizeof(hashLength),
                       &resultSize, 0);

    std::vector<UCHAR> hashObject(hashObjectSize);
    BCRYPT_HASH_HANDLE hash = nullptr;
    if (BCryptCreateHash(algorithm, &hash, hashObject.data(), hashObjectSize, nullptr, 0, 0) != 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        throw WebSocketError("BCryptCreateHashに失敗しました");
    }

    BCryptHashData(hash, reinterpret_cast<PUCHAR>(const_cast<char*>(input.data())),
                    static_cast<ULONG>(input.size()), 0);

    std::string digest(hashLength, '\0');
    BCryptFinishHash(hash, reinterpret_cast<PUCHAR>(digest.data()), hashLength, 0);

    BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    return digest;
}

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

// HTTPヘッダー行群を読み取り、キーを小文字に正規化したmapにする。
// リクエストライン/レスポンスラインは呼び出し側で別途読んでおくこと。
std::map<std::string, std::string> ReadHeaders(net::TcpConnection& connection) {
    std::map<std::string, std::string> headers;
    std::string line;
    while (connection.ReceiveLine(line) && !line.empty()) {
        const size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }
        headers[ToLower(Trim(line.substr(0, colonPos)))] = Trim(line.substr(colonPos + 1));
    }
    return headers;
}

std::string GenerateRandomWebSocketKey() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    std::string raw(16, '\0');
    for (char& c : raw) {
        c = static_cast<char>(dist(gen));
    }
    return Base64Encode(raw);
}

}  // namespace

std::string ComputeAcceptKey(const std::string& secWebSocketKey) {
    const std::string digest = Sha1(secWebSocketKey + kWebSocketGuid);
    return Base64Encode(digest);
}

void PerformServerHandshake(net::TcpConnection& connection) {
    std::string requestLine;
    if (!connection.ReceiveLine(requestLine)) {
        throw WebSocketError("ハンドシェイクリクエストを受信できませんでした");
    }
    // "GET /path HTTP/1.1" の形式だけを最小限確認する。
    if (requestLine.compare(0, 4, "GET ") != 0) {
        throw WebSocketError("GET以外のリクエストは非対応です: " + requestLine);
    }

    const std::map<std::string, std::string> headers = ReadHeaders(connection);
    const auto keyIt = headers.find("sec-websocket-key");
    if (keyIt == headers.end()) {
        throw WebSocketError("Sec-WebSocket-Keyヘッダーがありません");
    }

    const std::string acceptKey = ComputeAcceptKey(keyIt->second);

    std::ostringstream response;
    response << "HTTP/1.1 101 Switching Protocols\r\n";
    response << "Upgrade: websocket\r\n";
    response << "Connection: Upgrade\r\n";
    response << "Sec-WebSocket-Accept: " << acceptKey << "\r\n";
    response << "\r\n";
    connection.Send(response.str());
}

void PerformClientHandshake(net::TcpConnection& connection, const std::string& host, const std::string& path) {
    const std::string key = GenerateRandomWebSocketKey();

    std::ostringstream request;
    request << "GET " << path << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";
    request << "Upgrade: websocket\r\n";
    request << "Connection: Upgrade\r\n";
    request << "Sec-WebSocket-Key: " << key << "\r\n";
    request << "Sec-WebSocket-Version: 13\r\n";
    request << "\r\n";
    connection.Send(request.str());

    std::string statusLine;
    if (!connection.ReceiveLine(statusLine) || statusLine.find("101") == std::string::npos) {
        throw WebSocketError("ハンドシェイクに失敗しました(ステータス行: " + statusLine + ")");
    }

    const std::map<std::string, std::string> headers = ReadHeaders(connection);
    const auto acceptIt = headers.find("sec-websocket-accept");
    if (acceptIt == headers.end()) {
        throw WebSocketError("Sec-WebSocket-Acceptヘッダーがありません");
    }

    const std::string expected = ComputeAcceptKey(key);
    if (acceptIt->second != expected) {
        throw WebSocketError("Sec-WebSocket-Acceptの検証に失敗しました(期待値と不一致)");
    }
}

void SendTextFrame(net::TcpConnection& connection, const std::string& text, bool masked) {
    std::string frame;
    frame += static_cast<char>(0x80 | 0x1);  // FIN=1, opcode=0x1(テキスト)

    const uint64_t length = text.size();
    const unsigned char maskBit = masked ? 0x80 : 0x00;
    if (length <= 125) {
        frame += static_cast<char>(maskBit | static_cast<unsigned char>(length));
    } else if (length <= 0xFFFF) {
        frame += static_cast<char>(maskBit | 126);
        frame += static_cast<char>((length >> 8) & 0xFF);
        frame += static_cast<char>(length & 0xFF);
    } else {
        frame += static_cast<char>(maskBit | 127);
        for (int shift = 56; shift >= 0; shift -= 8) {
            frame += static_cast<char>((length >> shift) & 0xFF);
        }
    }

    if (masked) {
        unsigned char maskKey[4];
        std::random_device rd;
        for (unsigned char& b : maskKey) {
            b = static_cast<unsigned char>(rd() & 0xFF);
        }
        frame.append(reinterpret_cast<char*>(maskKey), 4);
        for (size_t i = 0; i < text.size(); ++i) {
            frame += static_cast<char>(static_cast<unsigned char>(text[i]) ^ maskKey[i % 4]);
        }
    } else {
        frame += text;
    }

    connection.Send(frame);
}

std::string ReceiveTextFrame(net::TcpConnection& connection) {
    const std::string header = connection.ReceiveExact(2);
    const unsigned char byte0 = static_cast<unsigned char>(header[0]);
    const unsigned char byte1 = static_cast<unsigned char>(header[1]);

    const bool fin = (byte0 & 0x80) != 0;
    const unsigned char opcode = byte0 & 0x0F;
    if (!fin) {
        throw WebSocketError("フラグメンテーションされたメッセージは非対応です");
    }
    if (opcode != 0x1) {
        throw WebSocketError("テキストフレーム以外(opcode=" + std::to_string(opcode) + ")は非対応です");
    }

    const bool masked = (byte1 & 0x80) != 0;
    uint64_t length = byte1 & 0x7F;
    if (length == 126) {
        const std::string ext = connection.ReceiveExact(2);
        length = (static_cast<unsigned char>(ext[0]) << 8) | static_cast<unsigned char>(ext[1]);
    } else if (length == 127) {
        const std::string ext = connection.ReceiveExact(8);
        length = 0;
        for (int i = 0; i < 8; ++i) {
            length = (length << 8) | static_cast<unsigned char>(ext[i]);
        }
    }

    unsigned char maskKey[4] = {0, 0, 0, 0};
    if (masked) {
        const std::string maskBytes = connection.ReceiveExact(4);
        for (int i = 0; i < 4; ++i) {
            maskKey[i] = static_cast<unsigned char>(maskBytes[i]);
        }
    }

    std::string payload = connection.ReceiveExact(static_cast<size_t>(length));
    if (masked) {
        for (size_t i = 0; i < payload.size(); ++i) {
            payload[i] = static_cast<char>(static_cast<unsigned char>(payload[i]) ^ maskKey[i % 4]);
        }
    }
    return payload;
}

}  // namespace ws
