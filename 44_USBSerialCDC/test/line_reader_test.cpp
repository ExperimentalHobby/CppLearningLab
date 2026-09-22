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

// 1回のreadChunk()で複数行分のデータが返ることがある(例:"foo\nbar\n")。
// 最初の改行より後ろを捨てると、そのデータが失われて2回目の呼び出しが
// 新しいreadChunk()を待ってタイムアウトしてしまい、以降の応答が
// ずれ続ける(PR #91のCopilotレビュー指摘)。
TEST(ReadLineTest, PreservesRemainderAfterNewlineForNextCall) {
    std::string pending;
    auto reader = MakeReader({"foo\nbar\n"});

    EXPECT_EQ(ReadLine(reader, pending), "foo\n");

    // 2回目はpendingBufferに残った"bar\n"だけで完結するはずなので、
    // readChunk()が呼ばれたら例外を投げるようにして「新たな読み取りに
    // 頼っていないこと」を検証する。
    auto failingReader = []() -> std::string {
        throw std::runtime_error("2回目の呼び出しでreadChunk()が呼ばれてはいけない");
    };
    EXPECT_EQ(ReadLine(failingReader, pending), "bar\n");
}

// 改行が来ないままデータが上限を超えて届いた場合、中途半端に打ち切ると
// 受信ストリームの残りが次のコマンドの応答と誤認され、以降の送受信が
// ずれ続けてしまう(PR #91のCopilotレビュー指摘)。ReadLineErrorを投げて
// 呼び出し側にセッション終了を促す。
TEST(ReadLineTest, ThrowsWhenLineExceedsMaxLengthWithoutNewline) {
    std::string pending;
    auto reader = MakeReader({std::string(5000, 'X')});  // 改行なしの長大なデータ

    EXPECT_THROW(ReadLine(reader, pending), ReadLineError);
}

// 改行を含むチャンクは、find('\n')が改行の存在を最優先で検出するため、
// 「改行が見つからない場合のみ上限を見る」実装だと上限チェックを回避できて
// しまう(例:5000バイトの改行なしデータの直後に'\n'が1つ来た場合)。
// 改行が見つかった場合でも、その1行自体の長さを上限と照合する必要がある
// (45_MicrocontrollerUSBCommのPR #93のCopilotレビュー指摘と同じ不具合)。
TEST(ReadLineTest, ThrowsWhenLineWithNewlineExceedsMaxLength) {
    std::string pending;
    auto reader = MakeReader({std::string(5000, 'X') + "\n"});  // 改行はあるが長すぎる1行

    EXPECT_THROW(ReadLine(reader, pending), ReadLineError);
}

// 上限チェックは「最初の1行」の長さに対して行うべきで、その後ろに
// たまたま大量の(次回以降処理する)データが控えていても、今回返す行が
// 短ければ例外にしてはいけない。
TEST(ReadLineTest, AllowsShortFirstLineEvenWithLargePendingRemainder) {
    std::string pending;
    auto reader = MakeReader({"OK\n" + std::string(5000, 'Y')});  // 短い1行+長い残り

    EXPECT_EQ(ReadLine(reader, pending), "OK\n");
}
