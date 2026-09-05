#include "file_transfer.h"

#include <gtest/gtest.h>

using namespace filexfer;

// 既知のSHA-256テストベクトル(PowerShellの[System.Security.Cryptography.SHA256]で
// 事前に算出して確認済みの値)。
TEST(ComputeSha256HexTest, MatchesKnownVectorForEmptyString) {
    EXPECT_EQ(ComputeSha256Hex(""), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST(ComputeSha256HexTest, MatchesKnownVectorForAbc) {
    EXPECT_EQ(ComputeSha256Hex("abc"), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST(ComputeSha256HexTest, DifferentInputsProduceDifferentHashes) {
    EXPECT_NE(ComputeSha256Hex("abc"), ComputeSha256Hex("abd"));
}

TEST(SplitIntoChunksTest, SplitsEvenlyDivisibleContent) {
    const auto chunks = SplitIntoChunks("AAAABBBB", 4);

    ASSERT_EQ(chunks.size(), 2u);
    EXPECT_EQ(chunks[0], "AAAA");
    EXPECT_EQ(chunks[1], "BBBB");
}

// 最後のチャンクはchunkSizeに満たなくてよい。
TEST(SplitIntoChunksTest, LastChunkCanBeShorter) {
    const auto chunks = SplitIntoChunks("AAAABB", 4);

    ASSERT_EQ(chunks.size(), 2u);
    EXPECT_EQ(chunks[0], "AAAA");
    EXPECT_EQ(chunks[1], "BB");
}

TEST(SplitIntoChunksTest, ReturnsEmptyVectorForEmptyContent) {
    EXPECT_TRUE(SplitIntoChunks("", 4).empty());
}

TEST(SplitIntoChunksTest, ThrowsForZeroChunkSize) {
    EXPECT_THROW(SplitIntoChunks("data", 0), FileTransferError);
}

TEST(FileStartEncodingTest, RoundTrips) {
    const std::string payload = EncodeFileStart("report.txt", 12345, "deadbeef");

    const FileStartInfo info = DecodeFileStart(payload);

    EXPECT_EQ(info.fileName, "report.txt");
    EXPECT_EQ(info.fileSize, 12345u);
    EXPECT_EQ(info.checksumHex, "deadbeef");
}

// 大きなファイルサイズ(32bitに収まらない値)でも正しく往復することを確認する。
TEST(FileStartEncodingTest, RoundTripsLargeFileSize) {
    const uint64_t largeSize = 5ULL * 1024 * 1024 * 1024;  // 5GiB
    const std::string payload = EncodeFileStart("big.bin", largeSize, "checksum");

    EXPECT_EQ(DecodeFileStart(payload).fileSize, largeSize);
}

TEST(FileStartEncodingTest, DecodeThrowsOnTruncatedPayload) {
    EXPECT_THROW(DecodeFileStart(std::string(1, '\0')), FileTransferError);
}

TEST(FileChunkEncodingTest, RoundTrips) {
    const std::string payload = EncodeFileChunk(7, "chunk-data");

    const FileChunkInfo info = DecodeFileChunk(payload);

    EXPECT_EQ(info.sequenceNumber, 7u);
    EXPECT_EQ(info.data, "chunk-data");
}

TEST(FileChunkEncodingTest, RoundTripsEmptyData) {
    const std::string payload = EncodeFileChunk(0, "");

    EXPECT_EQ(DecodeFileChunk(payload).data, "");
}

TEST(FileChunkEncodingTest, DecodeThrowsOnTruncatedPayload) {
    EXPECT_THROW(DecodeFileChunk("ab"), FileTransferError);
}

TEST(ReassembleChunksTest, ConcatenatesInSequenceOrder) {
    std::vector<FileChunkInfo> chunks = {
        {2, "CC"},
        {0, "AA"},
        {1, "BB"},
    };

    EXPECT_EQ(ReassembleChunks(std::move(chunks)), "AABBCC");
}

TEST(ReassembleChunksTest, ReturnsEmptyStringForNoChunks) {
    EXPECT_EQ(ReassembleChunks({}), "");
}
