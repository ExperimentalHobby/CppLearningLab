// 独自バイナリプロトコル(38_BinaryProtocolDesignのヘッダー+ペイロード形式を
// ファイル転送用に転用したもの): 「コマンドID + データ長 + ペイロード」形式。
//
// ヘッダーレイアウト(全てネットワークバイトオーダー、合計12バイト):
//   +----------------+---------+---------+----------+----------+
//   | magic (4bytes) | version | command | reserved | length   |
//   | "MYPB"         | 1 byte  | 1 byte  | 2 bytes  | 4 bytes  |
//   +----------------+---------+---------+----------+----------+
//   続けてlengthバイトのペイロード。
//
// FrameParserは、TCP/USBシリアルのストリーム特性(1回の送信が1回の受信に
// 対応する保証は無い)に対応するため、受信したバイト列を`Feed()`で蓄積し、
// ヘッダー+ペイロードが揃うたびに完成したMessageを1つずつ取り出す
// ストリーミングパーサーになっている。
#pragma once

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace proto {

class ProtocolError : public std::runtime_error {
   public:
    explicit ProtocolError(const std::string& message) : std::runtime_error(message) {}
};

enum class Command : uint8_t {
    kFileStart = 1,  // ペイロード: EncodeFileStartの形式(ファイル名+サイズ+チェックサム)
    kFileChunk = 2,  // ペイロード: EncodeFileChunkの形式(シーケンス番号+データ)
    kFileEnd = 3,    // ペイロード無し。送信側が全チャンク送信完了を通知する
    kAck = 4,        // ペイロード: 4byte(受信確認するシーケンス番号)
    kNack = 5,       // ペイロード: 4byte(再送を要求するシーケンス番号)
};

struct Message {
    Command command = Command::kFileStart;
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

}  // namespace proto
