#include "binary_protocol.h"

#include <winsock2.h>  // htonl/ntohl, htons/ntohs

#include <cstring>
#include <limits>

namespace proto {

namespace {

constexpr char kMagic[4] = {'M', 'Y', 'P', 'B'};
constexpr uint8_t kProtocolVersion = 1;
constexpr size_t kHeaderSize = 4 + 1 + 1 + 2 + 4;  // magic+version+command+reserved+length

// lengthフィールド(uint32_t)は信頼できない受信データなので、そのまま
// バッファを伸ばし続けるとメモリDoSになり得る。学習用途としては
// 十分な大きさを確保しつつ暴走を防げるよう、上限を明示的に設ける。
constexpr uint32_t kMaxPayloadSize = 4 * 1024 * 1024;  // 4MiB

}  // namespace

std::string Serialize(const Message& message) {
    std::string result;
    result.append(kMagic, sizeof(kMagic));
    result += static_cast<char>(kProtocolVersion);
    result += static_cast<char>(message.command);
    result += '\0';
    result += '\0';  // reserved(将来のフラグ用に予約、現状は未使用)

    // payload.size()はsize_t(64-bit環境ではuint32_tより広い)なので、
    // 4GiBを超える入力をそのままキャストすると桁あふれして不正なlengthに
    // なってしまう。事前に検証して拒否する。
    if (message.payload.size() > (std::numeric_limits<uint32_t>::max)()) {
        throw ProtocolError("ペイロードサイズがuint32_tの範囲を超えています: " +
                            std::to_string(message.payload.size()) + " bytes");
    }
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
        if (length > kMaxPayloadSize) {
            throw ProtocolError("ペイロード長が上限(" + std::to_string(kMaxPayloadSize) +
                                "bytes)を超えています: " + std::to_string(length) + " bytes");
        }

        if (buffer_.size() < kHeaderSize + length) {
            return;  // ペイロードがまだ全部届いていない。次のFeed()を待つ。
        }

        Message message;
        message.command = static_cast<Command>(command);
        message.payload = buffer_.substr(kHeaderSize, length);
        buffer_.erase(0, kHeaderSize + length);

        handler_(message);
        // ループを継続することで、1回のFeed()で複数メッセージが
        // 完成していた場合(TCP/USBシリアルで複数送信分がまとめて届いた場合)
        // にも対応する。
    }
}

}  // namespace proto
