#include "binary_protocol.h"

#include <winsock2.h>  // htonl/ntohl, htons/ntohs

#include <cstring>

namespace proto {

namespace {

constexpr char kMagic[4] = {'M', 'Y', 'P', 'B'};
constexpr uint8_t kProtocolVersion = 1;
constexpr size_t kHeaderSize = 4 + 1 + 1 + 2 + 4;  // magic+version+command+reserved+length

void AppendUint16(std::string& out, uint16_t value) {
    const uint16_t networkOrder = htons(value);
    out.append(reinterpret_cast<const char*>(&networkOrder), sizeof(networkOrder));
}

uint16_t ReadUint16(const std::string& data, size_t offset) {
    uint16_t networkOrder = 0;
    std::memcpy(&networkOrder, data.data() + offset, sizeof(networkOrder));
    return ntohs(networkOrder);
}

}  // namespace

std::string Serialize(const Message& message) {
    std::string result;
    result.append(kMagic, sizeof(kMagic));
    result += static_cast<char>(kProtocolVersion);
    result += static_cast<char>(message.command);
    result += '\0';
    result += '\0';  // reserved(将来のフラグ用に予約、現状は未使用)

    const uint32_t lengthNetworkOrder = htonl(static_cast<uint32_t>(message.payload.size()));
    result.append(reinterpret_cast<const char*>(&lengthNetworkOrder), sizeof(lengthNetworkOrder));

    result += message.payload;
    return result;
}

void FrameParser::Feed(const std::string& chunk) {
    buffer_ += chunk;
    ExtractCompleteFrames();
}

void FrameParser::ExtractCompleteFrames() {
    for (;;) {
        if (buffer_.size() < kHeaderSize) {
            return;  // ヘッダーすら揃っていない。次のFeed()を待つ。
        }

        if (buffer_.compare(0, sizeof(kMagic), kMagic, sizeof(kMagic)) != 0) {
            throw ProtocolError("マジックバイトが不正です(ストリームが同期していない可能性)");
        }

        const uint8_t version = static_cast<uint8_t>(buffer_[4]);
        if (version != kProtocolVersion) {
            throw ProtocolError("非対応のプロトコルバージョンです: " + std::to_string(version));
        }
        const uint8_t command = static_cast<uint8_t>(buffer_[5]);

        uint32_t lengthNetworkOrder = 0;
        std::memcpy(&lengthNetworkOrder, buffer_.data() + 8, sizeof(lengthNetworkOrder));
        const uint32_t length = ntohl(lengthNetworkOrder);

        if (buffer_.size() < kHeaderSize + length) {
            return;  // ペイロードがまだ全部届いていない。次のFeed()を待つ。
        }

        Message message;
        message.command = static_cast<Command>(command);
        message.payload = buffer_.substr(kHeaderSize, length);
        buffer_.erase(0, kHeaderSize + length);

        handler_(message);
        // ループを継続することで、1回のFeed()で複数メッセージが
        // 完成していた場合(TCPで複数送信分がまとめて届いた場合)にも対応する。
    }
}

std::string EncodeKeyValue(const std::string& key, const std::string& value) {
    std::string payload;
    AppendUint16(payload, static_cast<uint16_t>(key.size()));
    payload += key;
    AppendUint16(payload, static_cast<uint16_t>(value.size()));
    payload += value;
    return payload;
}

void DecodeKeyValue(const std::string& payload, std::string& outKey, std::string& outValue) {
    if (payload.size() < 2) {
        throw ProtocolError("kSetValueペイロードが短すぎます");
    }
    const uint16_t keyLen = ReadUint16(payload, 0);
    if (payload.size() < 2 + keyLen + 2) {
        throw ProtocolError("kSetValueペイロードのkey部分が不正です");
    }
    outKey = payload.substr(2, keyLen);
    const size_t valueLenOffset = 2 + keyLen;
    const uint16_t valueLen = ReadUint16(payload, valueLenOffset);
    if (payload.size() < valueLenOffset + 2 + valueLen) {
        throw ProtocolError("kSetValueペイロードのvalue部分が不正です");
    }
    outValue = payload.substr(valueLenOffset + 2, valueLen);
}

std::string EncodeKey(const std::string& key) {
    std::string payload;
    AppendUint16(payload, static_cast<uint16_t>(key.size()));
    payload += key;
    return payload;
}

std::string DecodeKey(const std::string& payload) {
    if (payload.size() < 2) {
        throw ProtocolError("kGetValueペイロードが短すぎます");
    }
    const uint16_t keyLen = ReadUint16(payload, 0);
    if (payload.size() < 2 + keyLen) {
        throw ProtocolError("kGetValueペイロードのkey部分が不正です");
    }
    return payload.substr(2, keyLen);
}

std::string EncodeValueResult(bool found, const std::string& value) {
    std::string payload;
    payload += static_cast<char>(found ? 1 : 0);
    if (found) {
        AppendUint16(payload, static_cast<uint16_t>(value.size()));
        payload += value;
    } else {
        AppendUint16(payload, 0);
    }
    return payload;
}

void DecodeValueResult(const std::string& payload, bool& outFound, std::string& outValue) {
    if (payload.size() < 3) {
        throw ProtocolError("kValueResultペイロードが短すぎます");
    }
    outFound = payload[0] != 0;
    const uint16_t valueLen = ReadUint16(payload, 1);
    if (payload.size() < 3 + valueLen) {
        throw ProtocolError("kValueResultペイロードのvalue部分が不正です");
    }
    outValue = outFound ? payload.substr(3, valueLen) : "";
}

}  // namespace proto
