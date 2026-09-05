#include "binary_protocol.h"

#include <gtest/gtest.h>

#include <vector>

#include "file_transfer.h"

using namespace proto;
using namespace filexfer;

TEST(SerializeAndFrameParserTest, RoundTripsFileStartMessage) {
    Message original;
    original.command = Command::kFileStart;
    original.payload = EncodeFileStart("test.txt", 100, "abc123");

    std::vector<Message> received;
    FrameParser parser([&](const Message& m) { received.push_back(m); });
    parser.Feed(Serialize(original));

    ASSERT_EQ(received.size(), 1u);
    EXPECT_EQ(received[0].command, Command::kFileStart);
    const FileStartInfo info = DecodeFileStart(received[0].payload);
    EXPECT_EQ(info.fileName, "test.txt");
}

// 1回のFeed()で複数メッセージ(start+chunk+end)が届いても、それぞれ1つずつ
// 正しく取り出せる(38番のFrameParserと同じストリーミング解析の仕組み)。
TEST(SerializeAndFrameParserTest, ExtractsFileStartChunkAndEndFromSingleFeed) {
    Message start;
    start.command = Command::kFileStart;
    start.payload = EncodeFileStart("a.bin", 4, "checksum");
    Message chunk;
    chunk.command = Command::kFileChunk;
    chunk.payload = EncodeFileChunk(0, "data");
    Message end;
    end.command = Command::kFileEnd;

    std::vector<Message> received;
    FrameParser parser([&](const Message& m) { received.push_back(m); });
    parser.Feed(Serialize(start) + Serialize(chunk) + Serialize(end));

    ASSERT_EQ(received.size(), 3u);
    EXPECT_EQ(received[0].command, Command::kFileStart);
    EXPECT_EQ(received[1].command, Command::kFileChunk);
    EXPECT_EQ(received[2].command, Command::kFileEnd);
}

// TCPやUSBシリアルのストリーム特性(1回の送信が1回の受信に対応する保証は
// 無い)を模して、メッセージを分割して与えても、全バイトが揃った時点で
// 正しく完成する。
TEST(SerializeAndFrameParserTest, AssemblesMessageSplitAcrossMultipleFeeds) {
    Message chunk;
    chunk.command = Command::kFileChunk;
    chunk.payload = EncodeFileChunk(3, "hello");
    const std::string serialized = Serialize(chunk);

    std::vector<Message> received;
    FrameParser parser([&](const Message& m) { received.push_back(m); });
    for (size_t i = 0; i + 1 < serialized.size(); ++i) {
        parser.Feed(serialized.substr(i, 1));
    }
    EXPECT_TRUE(received.empty());

    parser.Feed(serialized.substr(serialized.size() - 1, 1));
    ASSERT_EQ(received.size(), 1u);
}

TEST(FrameParserTest, ThrowsOnInvalidMagicBytes) {
    // Serializeを経由せず、意図的に壊れたヘッダーを直接与える。
    std::string badHeader = "XXXX";
    badHeader += static_cast<char>(1);
    badHeader += static_cast<char>(static_cast<uint8_t>(Command::kFileEnd));
    badHeader += '\0';
    badHeader += '\0';
    badHeader += std::string(4, '\0');  // length=0

    FrameParser parser([](const Message&) {});
    EXPECT_THROW(parser.Feed(badHeader), ProtocolError);
}

// エンドツーエンドの統合テスト: ファイル内容をチャンク分割してSerializeし、
// FrameParserで受信・再構成した結果が元の内容と一致することを確認する
// (実機が無いため、この自己完結テストでプロトコル全体の正しさを示す)。
TEST(EndToEndTransferTest, ReassembledContentMatchesOriginal) {
    const std::string original =
        "The quick brown fox jumps over the lazy dog. " "0123456789" "ABCDEFGHIJ";
    const std::string checksum = ComputeSha256Hex(original);
    const auto chunks = SplitIntoChunks(original, 16);

    std::string stream;
    Message start;
    start.command = Command::kFileStart;
    start.payload = EncodeFileStart("pangram.txt", original.size(), checksum);
    stream += Serialize(start);
    for (size_t i = 0; i < chunks.size(); ++i) {
        Message chunkMessage;
        chunkMessage.command = Command::kFileChunk;
        chunkMessage.payload = EncodeFileChunk(static_cast<uint32_t>(i), chunks[i]);
        stream += Serialize(chunkMessage);
    }
    Message end;
    end.command = Command::kFileEnd;
    stream += Serialize(end);

    std::vector<FileChunkInfo> receivedChunks;
    bool sawEnd = false;
    FrameParser parser([&](const Message& m) {
        if (m.command == Command::kFileChunk) {
            receivedChunks.push_back(DecodeFileChunk(m.payload));
        } else if (m.command == Command::kFileEnd) {
            sawEnd = true;
        }
    });
    parser.Feed(stream);

    EXPECT_TRUE(sawEnd);
    const std::string reassembled = ReassembleChunks(std::move(receivedChunks));
    EXPECT_EQ(reassembled, original);
    EXPECT_EQ(ComputeSha256Hex(reassembled), checksum);
}
