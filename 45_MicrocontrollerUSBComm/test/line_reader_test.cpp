#include "line_reader.h"

#include <deque>
#include <gtest/gtest.h>

using namespace serialcli;

namespace {

// テスト用の擬似readChunk: あらかじめ用意したチャンク列を順番に返し、
// 尽きたら空文字列(タイムアウト相当)を返し続ける。
std::function<std::string()> MakeReader(std::deque<std::string> chunks) {
    return [chunks = std::move(chunks)]() mutable -> std::string {
        if (chunks.empty()) {
            return "";
        }
        std::string chunk = std::move(chunks.front());
        chunks.pop_front();
        return chunk;
    };
}

}  // namespace

TEST(ReadLineTest, ReturnsLineFromSingleChunk) {
    std::string pending;
    auto reader = MakeReader({"OK\n"});

    EXPECT_EQ(ReadLine(reader, pending), "OK\n");
    EXPECT_TRUE(pending.empty());
}

TEST(ReadLineTest, AssemblesLineSplitAcrossMultipleChunks) {
    std::string pending;
    auto reader = MakeReader({"O", "K", "\n"});

    EXPECT_EQ(ReadLine(reader, pending), "OK\n");
}

TEST(ReadLineTest, ReturnsPartialLineOnTimeoutWithoutNewline) {
    std::string pending;
    auto reader = MakeReader({"partial"});  // この後は空文字列(タイムアウト)が続く

    EXPECT_EQ(ReadLine(reader, pending), "partial");
}

TEST(ReadLineTest, ReturnsEmptyStringWhenNoDataArrivesAtAll) {
    std::string pending;
    auto reader = MakeReader({});  // 最初からタイムアウト

    EXPECT_EQ(ReadLine(reader, pending), "");
}

// 1回のreadChunk()で複数行分のデータが返ることがある(例:"OK\nSENSOR:1\n")。
// 最初の改行より後ろを捨てると、そのデータが失われて2回目の呼び出しが
// 新しいreadChunk()を待ってタイムアウトしてしまい、以降の応答がずれ続ける
// (44_USBSerialCDCのPR #91のCopilotレビュー指摘と同じ不具合)。
TEST(ReadLineTest, PreservesRemainderAfterNewlineForNextCall) {
    std::string pending;
    auto reader = MakeReader({"OK\nSENSOR:1\n"});

    EXPECT_EQ(ReadLine(reader, pending), "OK\n");

    // 2回目はpendingBufferに残った"SENSOR:1\n"だけで完結するはずなので、
    // readChunk()が呼ばれたら例外を投げるようにして「新たな読み取りに
    // 頼っていないこと」を検証する。
    auto failingReader = []() -> std::string {
        throw std::runtime_error("2回目の呼び出しでreadChunk()が呼ばれてはいけない");
    };
    EXPECT_EQ(ReadLine(failingReader, pending), "SENSOR:1\n");
}

// 改行が来ないままデータが上限を超えて届いた場合、中途半端に打ち切ると
// 受信ストリームの残りが次のコマンドの応答と誤認され、以降の送受信が
// ずれ続けてしまう(44_USBSerialCDCのPR #91のCopilotレビュー指摘と同じ
// 不具合)。ReadLineErrorを投げて呼び出し側にセッション終了を促す。
TEST(ReadLineTest, ThrowsWhenLineExceedsMaxLengthWithoutNewline) {
    std::string pending;
    auto reader = MakeReader({std::string(5000, 'X')});  // 改行なしの長大なデータ

    EXPECT_THROW(ReadLine(reader, pending), ReadLineError);
}
