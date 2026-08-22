// 独自バイナリプロトコル: 「コマンドID + データ長 + ペイロード」形式。
//
// ヘッダーレイアウト(全てネットワークバイトオーダー、合計12バイト):
//   +----------------+---------+---------+----------+----------+
//   | magic (4bytes) | version | command | reserved | length   |
//   | "MYPB"         | 1 byte  | 1 byte  | 2 bytes  | 4 bytes  |
//   +----------------+---------+---------+----------+----------+
//   続けてlengthバイトのペイロード。
//
// FrameParserは、TCPのストリーム特性(1回のsend()が1回のrecv()に対応する
// 保証は無い)に対応するため、受信したバイト列を`Feed()`で蓄積し、
// ヘッダー+ペイロードが揃うたびに完成したMessageを1つずつ取り出す
// ストリーミングパーサーになっている。
#pragma once

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>

namespace proto {

class ProtocolError : public std::runtime_error {
   public:
    explicit ProtocolError(const std::string& message) : std::runtime_error(message) {}
};

enum class Command : uint8_t {
    kPing = 1,         // ペイロード無し
    kPong = 2,         // ペイロード無し
    kEcho = 3,         // ペイロード: 任意のバイト列(そのまま送り返される)
    kSetValue = 4,      // ペイロード: EncodeKeyValueの形式
    kGetValue = 5,      // ペイロード: EncodeKeyの形式
    kValueResult = 6,   // ペイロード: EncodeValueResultの形式
};

struct Message {
    Command command = Command::kPing;
    std::string payload;
};

// メッセージ1件分のバイト列(ヘッダー+ペイロード)を組み立てる。
std::string Serialize(const Message& message);

// 受信したバイト列を蓄積し、完成したメッセージをコールバックで1つずつ通知する
// ストリーミングパーサー。
class FrameParser {
   public:
    using MessageHandler = std::function<void(const Message&)>;

    explicit FrameParser(MessageHandler handler) : handler_(std::move(handler)) {}

    // chunkを内部バッファに追加し、完成したメッセージがあればhandlerを呼ぶ。
    // 1回のFeed()で複数メッセージが完成することもあれば、0件のこともある
    // (ヘッダーやペイロードが揃うまで待つ)。
    void Feed(const std::string& chunk);

   private:
    std::string buffer_;
    MessageHandler handler_;

    void ExtractCompleteFrames();
};

// kSetValueのペイロード形式: [2byte keyLen][key][2byte valueLen][value]
std::string EncodeKeyValue(const std::string& key, const std::string& value);
void DecodeKeyValue(const std::string& payload, std::string& outKey, std::string& outValue);

// kGetValueのペイロード形式: [2byte keyLen][key]
std::string EncodeKey(const std::string& key);
std::string DecodeKey(const std::string& payload);

// kValueResultのペイロード形式: [1byte found(0/1)][2byte valueLen][value]
// (found=0の場合、valueLenは0)
std::string EncodeValueResult(bool found, const std::string& value);
void DecodeValueResult(const std::string& payload, bool& outFound, std::string& outValue);

}  // namespace proto
