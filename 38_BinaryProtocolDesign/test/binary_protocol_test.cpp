#include "binary_protocol.h"

#include <winsock2.h>  // htonl

#include <gtest/gtest.h>

#include <vector>

using proto::Command;
using proto::DecodeKey;
using proto::DecodeKeyValue;
using proto::DecodeValueResult;
using proto::EncodeKey;
using proto::EncodeKeyValue;
using proto::EncodeValueResult;
using proto::FrameParser;
using proto::Message;
using proto::ProtocolError;
using proto::Serialize;

namespace {

// ヘッダーのmagic/versionを不正な値に差し替えた生バイト列を組み立てるヘルパー
// (FrameParserのエラー検出経路をテストするために使う)。
std::string BuildRawHeader(const char magic[4], uint8_t version, uint32_t length) {
    std::string header;
    header.append(magic, 4);
    header += static_cast<char>(version);
    header += static_cast<char>(proto::Command::kPing);
    header += '\0';
    header += '\0';
    const uint32_t net = htonl(length);
    header.append(reinterpret_cast<const char*>(&net), sizeof(net));
    return header;
}

}  // namespace

TEST(SerializeAndFrameParserTest, RoundTripsSingleMessage) {
    Message original;
    original.command = Command::kEcho;
    original.payload = "hello";

    std::vector<Message> received;
    FrameParser parser([&](const Message& m) { received.push_back(m); });
    parser.Feed(Serialize(original));

    ASSERT_EQ(received.size(), 1u);
    EXPECT_EQ(received[0].command, Command::kEcho);
    EXPECT_EQ(received[0].payload, "hello");
}

TEST(SerializeAndFrameParserTest, RoundTripsMessageWithEmptyPayload) {
    Message original;
    original.command = Command::kPing;

    std::vector<Message> received;
    FrameParser parser([&](const Message& m) { received.push_back(m); });
    parser.Feed(Serialize(original));

    ASSERT_EQ(received.size(), 1u);
    EXPECT_TRUE(received[0].payload.empty());
}

// TCPのストリーム特性(1回のsend()が1回のrecv()に対応する保証は無い)を模して、
// ヘッダーとペイロードを別々のFeed()に分割して渡しても、両方揃った時点で
// 1つのメッセージとして完成する。
TEST(SerializeAndFrameParserTest, AssemblesMessageSplitAcrossMultipleFeeds) {
    Message original;
    original.command = Command::kEcho;
    original.payload = "hello";
    const std::string serialized = Serialize(original);

    std::vector<Message> received;
    FrameParser parser([&](const Message& m) { received.push_back(m); });

    // 1バイトずつ分割して与えても、最後のバイトが届くまでメッセージは完成しない。
    for (size_t i = 0; i + 1 < serialized.size(); ++i) {
        parser.Feed(serialized.substr(i, 1));
    }
    EXPECT_TRUE(received.empty());

    parser.Feed(serialized.substr(serialized.size() - 1, 1));
    ASSERT_EQ(received.size(), 1u);
    EXPECT_EQ(received[0].payload, "hello");
}

// 1回のFeed()で複数メッセージ分のバイト列が届いた場合、両方とも1つずつ取り出される。
TEST(SerializeAndFrameParserTest, ExtractsMultipleMessagesFromSingleFeed) {
    Message first;
    first.command = Command::kEcho;
    first.payload = "one";
    Message second;
    second.command = Command::kEcho;
    second.payload = "two";

    std::vector<Message> received;
    FrameParser parser([&](const Message& m) { received.push_back(m); });
    parser.Feed(Serialize(first) + Serialize(second));

    ASSERT_EQ(received.size(), 2u);
    EXPECT_EQ(received[0].payload, "one");
    EXPECT_EQ(received[1].payload, "two");
}

TEST(FrameParserTest, ThrowsOnInvalidMagicBytes) {
    FrameParser parser([](const Message&) {});

    EXPECT_THROW(parser.Feed(BuildRawHeader("XXXX", 1, 0)), ProtocolError);
}

TEST(FrameParserTest, ThrowsOnUnsupportedVersion) {
    FrameParser parser([](const Message&) {});

    EXPECT_THROW(parser.Feed(BuildRawHeader("MYPB", 99, 0)), ProtocolError);
}

TEST(FrameParserTest, ThrowsWhenPayloadLengthExceedsLimit) {
    FrameParser parser([](const Message&) {});

    // kMaxPayloadSize(4MiB)を超えるlengthを名乗るヘッダーだけを渡す
    // (実際のペイロードデータは不要。ヘッダー検証の時点で例外になるはず)。
    EXPECT_THROW(parser.Feed(BuildRawHeader("MYPB", 1, 5 * 1024 * 1024)), ProtocolError);
}

TEST(EncodeDecodeKeyValueTest, RoundTrips) {
    const std::string payload = EncodeKeyValue("name", "Alice");

    std::string key;
    std::string value;
    DecodeKeyValue(payload, key, value);

    EXPECT_EQ(key, "name");
    EXPECT_EQ(value, "Alice");
}

TEST(EncodeDecodeKeyTest, RoundTrips) {
    const std::string payload = EncodeKey("name");

    EXPECT_EQ(DecodeKey(payload), "name");
}

TEST(EncodeDecodeValueResultTest, RoundTripsWhenFound) {
    const std::string payload = EncodeValueResult(true, "Alice");

    bool found = false;
    std::string value;
    DecodeValueResult(payload, found, value);

    EXPECT_TRUE(found);
    EXPECT_EQ(value, "Alice");
}

// found=falseの場合、valueは空として往復する。
TEST(EncodeDecodeValueResultTest, RoundTripsWhenNotFound) {
    const std::string payload = EncodeValueResult(false, "");

    bool found = true;
    std::string value = "leftover";
    DecodeValueResult(payload, found, value);

    EXPECT_FALSE(found);
    EXPECT_TRUE(value.empty());
}

TEST(DecodeKeyValueTest, ThrowsOnTruncatedPayload) {
    std::string key;
    std::string value;
    // keyLenフィールド(2バイト)にすら満たない1バイトだけのペイロード。
    EXPECT_THROW(DecodeKeyValue(std::string(1, '\0'), key, value), ProtocolError);
}
